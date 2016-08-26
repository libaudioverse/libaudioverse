//Turn off windows.h #defines for min and max.
#define NOMINMAX
#include "wasapi.hpp"
#include <audio_io/private/audio_outputs.hpp>
#include <audio_io/private/sample_format_converter.hpp>
#include <audio_io/private/latency_predictor.hpp>
#include <audio_io/private/logging.hpp>
#include <thread>
#include <atomic>
#include <chrono>
#include <string.h>
#include <windows.h>
#include <audioclient.h>

namespace audio_io {
namespace implementation {

/**Number of samples to write at once.*/
const int wasapi_chunk_length = 512;

WasapiOutputDevice::WasapiOutputDevice(std::function<void(float*, int)> callback, std::shared_ptr<IMMDevice> device, int inputFrames, int inputChannels, int inputSr, double minLatency, double startLatency, double maxLatency)  {
	this->device = device;
	logDebug("Attempting to initialize a Wasapi device.");
	IAudioClient* client_raw = nullptr;
	auto res = APARTMENTCALL(device->Activate, IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&client_raw);
	if(IS_ERROR(res)) {
		logDebug("Could not activate device.  Error code %i.", (int)res);
		throw AudioIOError("Wasapi: could not activate device.");
	}
	client = wrapComPointer(client_raw);
	WAVEFORMATEX *format = nullptr;
	res = APARTMENTCALL(client->GetMixFormat, &format);
	if(IS_ERROR(res)) {
		logDebug("Wasapi: could not get mix format. Error: %i", (int)res);
		throw AudioIOError("Wasapi: unable to retrieve mix format.");
	}
	if(format->wFormatTag == WAVE_FORMAT_EXTENSIBLE) this->format = *(WAVEFORMATEXTENSIBLE*)format;
	else this->format.Format = *format;
	CoTaskMemFree(format);
	//Msdn strongly hints that we will always be able to convert from float to the mix format in shared mode.
	//Consequently, force our format to be float and see if it's supported.
	//First, the WAVEFORMATEX.
	WAVEFORMATEX &f = this->format.Format;
	bool wasExtensible = f.wFormatTag == WAVE_FORMAT_EXTENSIBLE;
	f.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	f.nAvgBytesPerSec = 4*f.nSamplesPerSec*f.nChannels;
	f.nBlockAlign = f.nChannels*4;
	f.wBitsPerSample = 32;
	f.cbSize = 22;
	//Now the extended WAVEFORMATEXTENSIBLE.
	WAVEFORMATEXTENSIBLE &f2 = this->format;
	f2.Samples.wValidBitsPerSample = 32;
	f2.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
	if(wasExtensible == false) {
		//We need to do the channel mask, if we can.
		if(f.nChannels == 2) f2.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
		else if(f.nChannels == 4) f2.dwChannelMask = SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT;
		else if(f.nChannels == 6) f2.dwChannelMask = SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT;
		else if(f.nChannels == 8) f2.dwChannelMask = SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT;
		else f2.dwChannelMask = (1<<f.nChannels)-1; //At least have enough channels, even if they aren't the right ones.
	}
	res = APARTMENTCALL(client->IsFormatSupported, AUDCLNT_SHAREMODE_SHARED, (WAVEFORMATEX*)&(this->format), &format);
	if(IS_ERROR(res)) {
		logDebug("Requested mix format is not supported.  Attempt to use IEEE float failed. Error: %i", (int)res);
		throw AudioIOError("Wasapi: could not initialize with float audio..");
	}
	REFERENCE_TIME default_period, min_period;
	res = APARTMENTCALL(client->GetDevicePeriod, &default_period, &min_period);
	if(IS_ERROR(res)) {
		logDebug("Couldn't query device periods.  Error %i", res);
		throw AudioIOError("WASAPI: couldn't query device period.");
	}
	period = default_period;
	period_in_secs = (default_period*100.0)/1e9; //it's in 100 nanosecond units.
	logDebug("WASAPI: shared mode period is %f seconds", period_in_secs);
	//We ask for 100 MS of latency to play with.
	if(maxLatency < 0.1) maxLatency = 0.1;
	//And we maybe bump it up some more if it's less than twice the period in seconds.
	if(maxLatency < period_in_secs*2) maxLatency = period_in_secs*2;
	REFERENCE_TIME latencyNanoseconds = 1000000000*maxLatency;
	res = APARTMENTCALL(client->Initialize, AUDCLNT_SHAREMODE_SHARED, 0, latencyNanoseconds/100, 0, (WAVEFORMATEX*)&(this->format), NULL);
	if(res != S_OK) {
		logDebug("Call to IAudioClient::initialize failed. COM error %i.", (int)res);
		throw AudioIOError("Wasapi: call to IAudioClient::initialize failed.");
	}
	//Get the buffer size and use it to make the predictor.
	UINT32 bufferSize;
	res = APARTMENTCALL(client->GetBufferSize, &bufferSize);
	if(IS_ERROR(res)) {
		logDebug("Attempt to get buffer size failed with error %i", (int)res);
		throw AudioIOError("Couldn't get Wasapi buffer size.");
	}
	wasapi_buffer_size = bufferSize;
	logDebug("Wasapi buffer size: %i", wasapi_buffer_size);
	int outputSr = this->format.Format.nSamplesPerSec;
	//We need to be a bit more than a period or we will fail.
	//maxLatency is always at least 2 periods.
	if(minLatency < period_in_secs*1.01) minLatency = period_in_secs*1.01;
	if(maxLatency > bufferSize/(float)outputSr) maxLatency = bufferSize/(float)outputSr;
	//Clamp starting latency.
	if(startLatency < minLatency) startLatency=  minLatency;
	if(startLatency > maxLatency) startLatency = maxLatency;
	logDebug("minLatency=%f, startLatency=%f, maxLatency=%f, bufferSize=%i, output_sr=%i", minLatency, startLatency, maxLatency, (int)bufferSize, outputSr);
	latency_predictor = new LatencyPredictor(30, minLatency, startLatency, maxLatency);
	init(callback, inputFrames, inputChannels, inputSr, this->format.Format.nChannels, outputSr);
	//At this point, we no longer need to go via the STA for the client interface.
	should_continue.test_and_set();
	wasapi_mixing_thread = std::thread(&WasapiOutputDevice::wasapiMixingThreadFunction, this);
	logDebug("Initialized Wasapi device.");
}

WasapiOutputDevice::~WasapiOutputDevice() {
	stop();
	delete latency_predictor;
}

void WasapiOutputDevice::stop() {
	if(stopped) return;
	logInfo("Wasapi device shutting down.");
	logDebug("Stopping a Wasapi device.");
	should_continue.clear();
	wasapi_mixing_thread.join();
	stopped = true;
}

void WasapiOutputDevice::wasapiMixingThreadFunction() {
	//Stuff here can run outside the apartment.
	auto res = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if(IS_ERROR(res)) {
		logDebug("Wassapi device mixing thread: could not initialize COM. Error %i", (int)res);
		return; //We really can't recover from this.
	}
	IAudioRenderClient *renderClient_raw = nullptr;
	UINT32 padding, bufferSize;
	client->GetBufferSize(&bufferSize);
	client->GetCurrentPadding(&padding);
	client->GetService(IID_IAudioRenderClient, (void**)&renderClient_raw);
	auto renderClient = wrapComPointer(renderClient_raw);
	//We use double buffering, as processing can take a long time.
	//MSDN warns us not to do intensive processing between GetBuffer and ReleaseBuffer.
	float* workspace = new float[output_channels*bufferSize]();
	BYTE* audioBuffer = nullptr;
	sample_format_converter->write(bufferSize-padding, workspace);
	renderClient->GetBuffer(bufferSize-padding, &audioBuffer);
	memcpy(audioBuffer, workspace, sizeof(float)*output_channels*(bufferSize-padding));
	renderClient->ReleaseBuffer(bufferSize-padding, 0);
	//The buffer is filled, so we begin processing.
	client->Start();
	logDebug("Wasapi mixing thread: audio client is started.  Mixing audio.");
	bool workspaceContainsChunk = false;
	while(should_continue.test_and_set()) {
		//Get the number of frames we want before continuing.
		double targetLatency = latency_predictor->predictLatency();
		int targetLatencyFrames = (int)(output_sr*targetLatency);
		//Predicted latency can go too high for us to write anything, so clamp it at the buffer size less half a period.
		int targetLatencyFramesMax = (int)(wasapi_buffer_size-period_in_secs*output_sr*0.5);
		int targetPadding = std::min(targetLatencyFrames, targetLatencyFramesMax);
		client->GetCurrentPadding(&padding);
		//Wait until we have enough data.
		if(padding > targetPadding) {
			int sleepFrames = (targetPadding-padding)/2;
			int sleepMs = sleepFrames*1000/output_sr;
			if(sleepMs > 0) std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
			continue;
		}
		latency_predictor->beginPass();
		if(workspaceContainsChunk == false) sample_format_converter->write(wasapi_chunk_length, workspace);
		workspaceContainsChunk = true;
		if(renderClient->GetBuffer(wasapi_chunk_length, &audioBuffer) != S_OK) {
			latency_predictor->endPass();
			std::this_thread::yield();
			continue;
		}
		memcpy(audioBuffer, workspace, sizeof(float)*wasapi_chunk_length*output_channels);
		renderClient->ReleaseBuffer(wasapi_chunk_length, 0);
		workspaceContainsChunk = false;
		latency_predictor->endPass();
	}
	client->Stop();
	client->Reset();
	delete[] workspace;
	CoUninitialize();
	logDebug("Wasapi mixing thread: exiting.");
}

}
}