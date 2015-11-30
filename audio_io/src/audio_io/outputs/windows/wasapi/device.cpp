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
	//We ask for 100 MS of latency to play with.
	REFERENCE_TIME latencyNanoseconds = 100000000;
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
	int outputSr = this->format.Format.nSamplesPerSec;
	if(minLatency < wasapi_chunk_length/(float)outputSr) minLatency = wasapi_chunk_length/(float)outputSr;
	if(maxLatency > bufferSize/(float)outputSr) maxLatency = bufferSize/(float)outputSr;
	//Clamp starting latency.
	startLatency = std::min(std::max(startLatency, minLatency), maxLatency);
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
	//MSDN warns us not to not do intensive processing between GetBuffer and ReleaseBuffer.
	float* workspace = new float[output_channels*bufferSize]();
	BYTE* audioBuffer = nullptr;
	sample_format_converter->write(bufferSize-padding, workspace);
	renderClient->GetBuffer(bufferSize-padding, &audioBuffer);
	memcpy(audioBuffer, workspace, sizeof(float)*output_channels*(bufferSize-padding));
	renderClient->ReleaseBuffer(bufferSize-padding, 0);
	//The buffer is filled, so we begin processing.
	client->Start();
	logDebug("Wasapi mixing thread: audio client is started.  Mixing audio.");
	//From here, it's thankfully much simpler.  Every time we have at least output_frames worth of empty buffer, we fill it.
	bool workspaceContainsChunk = false;
	while(should_continue.test_and_set()) {
		//Get the number of frames we want before continuing.
		double targetLatency = latency_predictor->predictLatency();
		int targetLatencyFrames = (int)(output_sr*targetLatency);
		//We do this to make sure that we always write something.
		//Predicted latency can go too high for us to write a  chunk.
		int targetPadding = std::min(bufferSize-wasapi_chunk_length, bufferSize-targetLatencyFrames);
		client->GetCurrentPadding(&padding);
		//Wait until we have enough data.
		if(padding > targetPadding) {
			std::this_thread::sleep_for(std::chrono::milliseconds((padding-targetPadding)*1000/output_sr));
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