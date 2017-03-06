//Turn off windows.h #defines for min and max.
#define NOMINMAX
#include "wasapi.hpp"
#include <audio_io/private/audio_outputs.hpp>
#include <audio_io/private/output_worker_thread.hpp>
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

WasapiOutputDevice::WasapiOutputDevice(std::function<void(float*, int)> callback, std::shared_ptr<IMMDevice> device, int inputFrames, int inputChannels, int inputSr, int mixahead)  {
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
	res = APARTMENTCALL(client->Initialize, AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, 0, 0, (WAVEFORMATEX*)&(this->format), NULL);
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
	init(callback, inputFrames, inputChannels, inputSr, this->format.Format.nChannels, outputSr, mixahead);
	//At this point, we no longer need to go via the STA for the client interface.
	should_continue.test_and_set();
	wasapi_mixing_thread = std::thread(&WasapiOutputDevice::wasapiMixingThreadFunction, this);
	logDebug("Initialized Wasapi device.");
}

WasapiOutputDevice::~WasapiOutputDevice() {
	stop();
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
	// Wait on the initial mix.
	worker_thread->awaitInitialMix();
	//Stuff here can run outside the apartment.
	auto res = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if(IS_ERROR(res)) {
		logDebug("Wassapi device mixing thread: could not initialize COM. Error %i", (int)res);
		return; //We really can't recover from this.
	}
	IAudioRenderClient *renderClient_raw = nullptr;
	UINT32 padding, bufferSize;
	client->GetBufferSize(&bufferSize);
	client->GetService(IID_IAudioRenderClient, (void**)&renderClient_raw);
	auto renderClient = wrapComPointer(renderClient_raw);
	HANDLE event = CreateEvent(NULL, FALSE, FALSE, NULL);
	client->SetEventHandle(event);
	// We need to output whatever we can, without having to zero the rest of the packet for Wasapi.
	float* intermediateStorage = new float[bufferSize*output_channels];
	std::fill(intermediateStorage, intermediateStorage+bufferSize*output_channels, 0.0f);
	client->Start();
	logDebug("Wasapi mixing thread: audio client is started.  Mixing audio.");
	while(should_continue.test_and_set()) {
		padding = 0;
		client->GetCurrentPadding(&padding);
		// We have the padding. This value cannot decrease while we write, only increase.
		// So go to intermediateStorage, and figure out how much we have to request.
		int toRequest = bufferSize-padding;
		toRequest = worker_thread->write(toRequest, intermediateStorage);
		if(toRequest == 0) continue;
		BYTE* audioBuffer;
		if(renderClient->GetBuffer(toRequest, &audioBuffer) == S_OK) {
			memcpy(audioBuffer, intermediateStorage, sizeof(float)*output_channels*toRequest);
			renderClient->ReleaseBuffer(toRequest, 0);
		}
		WaitForSingleObject(event, 0);
	}
	client->Stop();
	client->Reset();
	CoUninitialize();
	CloseHandle(event);
	delete[] intermediateStorage;
	logDebug("Wasapi mixing thread: exiting.");
}

}
}