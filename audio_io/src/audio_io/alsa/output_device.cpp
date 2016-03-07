#include "alsa.hpp"
#include <audio_io/audio_io.hpp>
#include <audio_io/private/audio_outputs.hpp>
#include <audio_io/private/sample_format_converter.hpp>
#include <vector>
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <algorithm>
#include <alsa/asoundlib.h>

namespace audio_io {
namespace implementation {

AlsaOutputDevice::AlsaOutputDevice(std::function<void(float*, int)> callback, std::string name, int sr, int channels, int blockSize, float minLatency, float startLatency, float maxLatency) {
	snd_pcm_hw_params_t *params;
	snd_pcm_sw_params_t *params_sw;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_sw_params_alloca(&params_sw);
	int res = snd_pcm_open(&device_handle, name.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
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
	res = snd_pcm_hw_params_set_rate_near(device_handle, params, &alsaSr, 0);
	if(res < 0) {
		throw AudioIOError("ALSA: Couldn't constrain sample rate.");
	}
	res = snd_pcm_hw_params_set_access(device_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if(res < 0) {
		throw AudioIOError("ALSA: couldn't set access.");
	}
	//Use 10 milliseconds (1 ms = 1000 us).
	//No app is likely to go below this, nor should we.
	unsigned int alsaPeriodMicrosecs = 10000;
	res = snd_pcm_hw_params_set_period_time_near(device_handle, params, &alsaPeriodMicrosecs, 0);
	if(res < 0) {
		throw AudioIOError("ALSA: couldn't configure period time.");
	}
	//compute needed buffer size and bump maxLatency if needed.
	maxLatency = std::max<double>(maxLatency, 2.3*blockSize/(double)sr);
	snd_pcm_uframes_t alsaBufferSize = alsaSr*maxLatency;
	res = snd_pcm_hw_params_set_buffer_size_near(device_handle, params, &alsaBufferSize);
	if(res < 0) {
		throw AudioIOError("ALSA: couldn't set buffer size.");
	}
	res = snd_pcm_hw_params(device_handle, params);
	if(res < 0) {
		throw AudioIOError("ALSA: Could not prepare device.");
	}
	//Software parameter setup.
	res = snd_pcm_sw_params_current(device_handle, params_sw);
	if(res < 0) {
		throw AudioIOError("ALSA: couldn't get current software parameters.");
	}
	//Start the device when the buffer is 90% full.
	res = snd_pcm_sw_params_set_start_threshold(device_handle, params_sw, (snd_pcm_uframes_t)(0.9*alsaBufferSize));
	if(res < 0) {
		throw AudioIOError("ALSA: couldn't set startup threshold.");
	}
	res = snd_pcm_sw_params(device_handle, params_sw);
	if(res < 0) {
		throw AudioIOError("ALSA: couldn't install software params.");
	}
	init(callback, blockSize, channels, sr, (int)alsaChannels, (int)alsaSr);
	worker_running.test_and_set();
	worker_thread = std::thread([&] () {workerThreadFunction();});
}

AlsaOutputDevice::~AlsaOutputDevice() {
	stop();
}

void AlsaOutputDevice::stop() {
	if(stopped == false) {
		if(worker_thread.joinable()) {
			worker_running.clear();
			worker_thread.join();
		}
	}
	stopped = true;
}

void AlsaOutputDevice::workerThreadFunction() {
	float* buffer = new float[output_channels*output_frames]();
	while(worker_running.test_and_set()) {
		sample_format_converter->write(output_frames, buffer);
		snd_pcm_sframes_t res = snd_pcm_writei(device_handle, buffer, (snd_pcm_uframes_t)output_frames);
		if(res < 0) {
			snd_pcm_prepare(device_handle);
		}
	}
	snd_pcm_drain(device_handle);
	snd_pcm_close(device_handle);
	delete[] buffer;
}

}
}