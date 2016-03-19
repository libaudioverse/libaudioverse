#include "alsa.hpp"
#include <audio_io/audio_io.hpp>
#include <audio_io/private/audio_outputs.hpp>
#include <audio_io/private/sample_format_converter.hpp>
#include <audio_io/private/latency_predictor.hpp>
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

AlsaOutputDevice::AlsaOutputDevice(std::function<void(float*, int)> callback, std::string name, int sr, int channels, int blockSize, float minLatency, float startLatency, float maxLatency) {
	snd_pcm_hw_params_t *params;
	snd_pcm_sw_params_t *params_sw;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_sw_params_alloca(&params_sw);
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
	res = snd_pcm_hw_params_set_rate_near(device_handle, params, &alsaSr, 0);
	if(res < 0) {
		throw AudioIOError("ALSA: Couldn't constrain sample rate.");
	}
	res = snd_pcm_hw_params_set_access(device_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if(res < 0) {
		throw AudioIOError("ALSA: couldn't set access.");
	}
	//Use 3 milliseconds (1 ms = 1000 us).
	//No app is likely to go below this, nor should we.
	unsigned int alsaPeriodMicrosecs = 3000;
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
	alsa_buffer_frames = (int)alsaBufferSize;
	//minLatency needs to be at least half a block.
	minLatency = std::max<double>(minLatency, blockSize/2.0/sr);
	//it is theoretically possible that the Alsa buffer is smaller than our minimum latency by enough to make us never write.
	minLatency = std::min<float>(minLatency, alsaBufferSize/(double)alsaSr/2.0);
	//But if we're less than a period, that's bad.
	minLatency = std::max<double>(minLatency, alsaPeriodMicrosecs/(double)1000000+0.001);
	//And we may need to adjust maxLatency.
	//We need to ensure that we have at least one sample, so we use 95% of the buffer.
	maxLatency = std::min<float>(maxLatency, 0.95*alsaBufferSize/(double)sr);
	//maxLatency must be at least minLatency.
	maxLatency = std::max(maxLatency, minLatency);
	//startLatency needs to be between them.
	startLatency = std::min(startLatency, maxLatency);
	startLatency = std::max(startLatency, minLatency);
	res = snd_pcm_hw_params(device_handle, params);
	if(res < 0) {
		throw AudioIOError("ALSA: Could not prepare device.");
	}
	//Software parameter setup.
	res = snd_pcm_sw_params_current(device_handle, params_sw);
	if(res < 0) {
		throw AudioIOError("ALSA: couldn't get current software parameters.");
	}
	//Start the device when the buffer is over the minimum latency.
	res = snd_pcm_sw_params_set_start_threshold(device_handle, params_sw, std::max<int>(minLatency*alsaSr-1, 0));
	if(res < 0) {
		throw AudioIOError("ALSA: couldn't set startup threshold.");
	}
	//We want to delay stopping as much as we can.
	//To that end, we set the stop threshold to 1 less than the buffer.
	//It appears that Alsa has interesting and difficult subtleties, should we disable this.  We avoid that for now.
	res = snd_pcm_sw_params_set_stop_threshold(device_handle, params_sw, alsaBufferSize-1);
	if(res < 0) {
		throw AudioIOError("ALSA: Couldn't set stop threshold.");
	}
	res = snd_pcm_sw_params(device_handle, params_sw);
	if(res < 0) {
		throw AudioIOError("ALSA: couldn't install software params.");
	}
	latency_predictor = new LatencyPredictor(30, minLatency, startLatency, maxLatency);
	//Keep this block.  It's very useful to be able to uncomment it for debugging.
	/*
	snd_output_t *output = nullptr;
	snd_output_stdio_attach(&output, stdout, 0);
	snd_pcm_dump(device_handle, output);
	*/
	init(callback, blockSize, channels, sr, (int)alsaChannels, (int)alsaSr);
	worker_running.test_and_set();
	worker_thread = std::thread([&] () {workerThreadFunction();});
}

AlsaOutputDevice::~AlsaOutputDevice() {
	stop();
	delete latency_predictor;
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
	int retries = 0;
	const int retryLimit = 10;
	while(worker_running.test_and_set()) {
		double targetLatency = latency_predictor->predictLatency();
		bool forceWrite = false;
		bool hadUnderrun = false;
		auto avail = snd_pcm_avail(device_handle);
		if(avail < 0) {
			auto res = snd_pcm_recover(device_handle, avail, 1);
			if(avail == -EPIPE) {
				forceWrite = true;
				latency_predictor->hadUnderrun();
				continue;
			}
			retries++;
			//protect against possibly getting into a potentially unrecoverable state.
			if(res < 0 || retries >= retryLimit) break;
			continue;
		}
		retries = 0;
		double deviceLatency = (alsa_buffer_frames-avail)/(double)output_sr;
		if(deviceLatency > targetLatency && forceWrite == false) {
			double sleepFor = targetLatency-deviceLatency;
			int sleepForMs = sleepFor*1000;
			if(sleepForMs == 0) continue;
			else std::this_thread::sleep_for(std::chrono::milliseconds(sleepForMs));
		}
		else {
			latency_predictor->beginPass();
			sample_format_converter->write(output_frames, buffer);
			//If we're 5.1 or 7.1, we need to swap the channels around to match Linux's idea of surround sound.
			//The stuff here comes from http://drona.csa.iisc.ernet.in/~uday/alsamch.shtml
			//If this doesn't work, there is an ALSA channel mapping API.
			if(output_channels == 6 || output_channels == 8) {
				const int initial_center = 2, initial_lfe = 3;
				const int new_center = output_channels-2, new_lfe = output_channels-1;
				for(int i = 0; i < output_frames*output_channels; i+= output_channels) {
					std::swap(buffer[i+initial_center], buffer[i+new_center]);
					std::swap(buffer[i+initial_lfe], buffer[i+new_lfe]);
				}
			}
			int handled = 0;
			while(handled < output_frames) {
				snd_pcm_sframes_t res = snd_pcm_writei(device_handle, buffer+handled*output_channels, (snd_pcm_uframes_t)(output_frames-handled));
				if(res == -EAGAIN) continue;
				if(res < 0) {
					if(res == -EPIPE) hadUnderrun = true;
					res = snd_pcm_recover(device_handle, res, 1);
					//if res is still less than zero, fail out.
					if(res < 0) goto cleanup;
				}
				handled += res;
			}
			latency_predictor->endPass();
			if(hadUnderrun) {
				latency_predictor->hadUnderrun();
			}
		}
	}
	cleanup:
	snd_pcm_drain(device_handle);
	snd_pcm_close(device_handle);
	delete[] buffer;
}

}
}