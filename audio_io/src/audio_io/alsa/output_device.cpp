#include "alsa.hpp"
#include <audio_io/audio_io.hpp>
#include <audio_io/private/audio_outputs.hpp>
#include <audio_io/private/output_worker_thread.hpp>
#include <audio_io/private/logging.hpp>
#include <vector>
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <algorithm>
#include <chrono>
#include <alsa/asoundlib.h>
#include <errno.h>
#include <stdio.h>

namespace audio_io {
namespace implementation {

AlsaOutputDevice::AlsaOutputDevice(std::function<void(float*, int)> callback, std::string name, int sr, int channels, int blockSize, int mixahead) {
	snd_pcm_hw_params_t *params;
	snd_pcm_sw_params_t *params_sw;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_sw_params_alloca(&params_sw);
	int dir = 0;
	int res = snd_pcm_open(&device_handle, name.c_str(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	if(res) {
		throw AudioIOError("An Alsa Error occurred.");
	}
	res = snd_pcm_hw_params_any(device_handle, params);
	if(res < 0) {
		throw AudioIOError("Alsa: could not configuer PCM device.");
	}
	//TODO: this doesn't deal with endianness properly, and uses our knowledge that x86 is little endian.
	res = snd_pcm_hw_params_set_format(device_handle, params, SND_PCM_FORMAT_FLOAT_LE);
	if(res < 0) {
		throw AudioIOError("ALSA: Couldn't get little endian float.");
	}
	unsigned int alsaChannels = channels;
	res = snd_pcm_hw_params_set_channels_near (device_handle, params, &alsaChannels);
	if(res < 0) {
		throw AudioIOError("Couldn't constrain channel count.");
	}
	unsigned int alsaSr = sr;
	res = snd_pcm_hw_params_set_rate_near(device_handle, params, &alsaSr, &dir);
	if(res < 0) {
		throw AudioIOError("ALSA: Couldn't constrain sample rate.");
	}
	res = snd_pcm_hw_params_set_access(device_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if(res < 0) {
		throw AudioIOError("ALSA: couldn't set access.");
	}
	snd_pcm_uframes_t alsaPeriodSize = 2048;
	res = snd_pcm_hw_params_set_period_size_near(device_handle, params, &alsaPeriodSize, &dir);
	if(res < 0) {
		throw AudioIOError("ALSA: Couldn't set period size.");
	}
	logDebug("Alsa period size: %i", alsaPeriodSize);
	snd_pcm_uframes_t alsaBufferSize = 2*alsaPeriodSize;
	// To first multiple larger than 8192.
	while(alsaBufferSize < 8192) alsaBufferSize += alsaPeriodSize;
	res = snd_pcm_hw_params_set_buffer_size_first(device_handle, params, &alsaBufferSize);
	if(res < 0) {
		throw AudioIOError("ALSA: couldn't set buffer size.");
	}
	logDebug("Alsa buffer size: %i", alsaBufferSize);
	alsa_buffer_frames = (int)alsaBufferSize;
	res = snd_pcm_hw_params(device_handle, params);
	if(res < 0) {
		throw AudioIOError("ALSA: Could not prepare device.");
	}
	//Software parameter setup.
	res = snd_pcm_sw_params_current(device_handle, params_sw);
	if(res < 0) {
		throw AudioIOError("ALSA: couldn't get current software parameters.");
	}
	res = snd_pcm_sw_params_set_start_threshold(device_handle, params_sw, alsaBufferSize);
	if(res < 0) {
		throw AudioIOError("ALSA: couldn't set startup threshold.");
	}
	res = snd_pcm_sw_params_set_silence_threshold(device_handle, params_sw, alsaPeriodSize);
	if(res < 0) {
		throw AudioIOError("ALSA: Couldn't set silence threshold.");
	}
	res = snd_pcm_sw_params_set_silence_size(device_handle, params_sw, alsaBufferSize);
	if(res < 0) {
		throw AudioIOError("ALSA: couldn't set silence size.");
	}
	// How often to wake us up.
	res = snd_pcm_sw_params_set_avail_min(device_handle, params_sw, alsaPeriodSize);
	if(res < 0) {
		throw AudioIOError("ALSA: couldn't set avail_min");
	}
	res = snd_pcm_sw_params(device_handle, params_sw);
	if(res < 0) {
		throw AudioIOError("ALSA: couldn't install software params.");
	}
	//Keep this block.  It's very useful to be able to uncomment it for debugging.
	/*
	snd_output_t *output = nullptr;
	snd_output_stdio_attach(&output, stdout, 0);
	snd_pcm_dump(device_handle, output);
	*/
	init(callback, blockSize, channels, sr, (int)alsaChannels, (int)alsaSr, mixahead);
	worker_running.test_and_set();
	io_thread = std::thread([&] () {workerThreadFunction();});
}

AlsaOutputDevice::~AlsaOutputDevice() {
	stop();
}

void AlsaOutputDevice::stop() {
	if(stopped == false) {
		if(io_thread.joinable()) {
			worker_running.clear();
			io_thread.join();
		}
	}
	stopped = true;
}

#define ALSAREC(err) do { \
if(err < 0) {\
if(snd_pcm_recover(device_handle, err, 1) < 0) {\
logDebug("ALSA: audio I/O thread encountered unrecoverable error %i: %s", err, snd_strerror(err));\
goto CLEANUP;\
}\
}} while(0)

void AlsaOutputDevice::workerThreadFunction() {
	worker_thread->awaitInitialMix();
	float* workspace = new float[alsa_buffer_frames*output_channels];
	std::fill(workspace, workspace+alsa_buffer_frames*output_channels, 0.0f);
	while(worker_running.test_and_set()) {
		auto available = snd_pcm_avail_update(device_handle);
		ALSAREC(available);
		if(available < 0) continue;
		if(available > alsa_buffer_frames) available = alsa_buffer_frames;
		int written = worker_thread->write(available, workspace);
		//If we're 5.1 or 7.1, we need to swap the channels around to match Linux's idea of surround sound.
		//The stuff here comes from http://drona.csa.iisc.ernet.in/~uday/alsamch.shtml
		//If this doesn't work, there is an ALSA channel mapping API.
		if(output_channels == 6 || output_channels == 8) {
			const int initial_center = 2, initial_lfe = 3;
			const int new_center = output_channels-2, new_lfe = output_channels-1;
			for(int i = 0; i < written*output_channels; i+= output_channels) {
				std::swap(workspace[i+initial_center], workspace[i+new_center]);
				std::swap(workspace[i+initial_lfe], workspace[i+new_lfe]);
			}
		}
		auto res = snd_pcm_writei(device_handle, workspace, written);
		ALSAREC(res);
		res = snd_pcm_wait(device_handle, 200);
		ALSAREC(res);
	}
	snd_pcm_drain(device_handle);
	snd_pcm_close(device_handle);
	CLEANUP:
	delete[] workspace;
}

}
}