#include <audio_io/audio_io.hpp>
#include <audio_io/private/sample_format_converter.hpp>
#include <speex_resampler_cpp.hpp>
#include <memory>
#include<functional>
#include <algorithm>


namespace audio_io {
namespace implementation {

SampleFormatConverter::SampleFormatConverter(std::function<void(float*, int)> callback, int inputFrames, int inputChannels, int inputSr, int outputChannels, int outputSr):
input_frames(inputFrames), input_sr(inputSr), input_channels(inputChannels),
output_sr(outputSr), output_channels(outputChannels) {
	output_frames = input_frames; //the default, may get overridden in a second.
	if(inputSr != outputSr) {
		resampler = speex_resampler_cpp::createResampler(inputFrames, inputChannels, inputSr, outputSr);
		int resamplerLength = inputFrames*outputSr/inputSr;
		if(resamplerLength == 0) resamplerLength = inputFrames;
		resampler_workspace = new float[resamplerLength*inputChannels]();
		output_frames = resamplerLength;
	}
	input_buffer = new float[input_frames*input_channels]();
	output_buffer = new float[output_frames*output_channels]();
	consumed_output_frames = output_frames; //we have none available yet.
	this->callback = callback;
}

SampleFormatConverter::~SampleFormatConverter() {
	delete[] output_buffer;
	delete[] input_buffer;
	if(resampler_workspace) delete[] resampler_workspace;
}

void SampleFormatConverter::write(int frames, float* buffer) {
	int written = 0;
	while(written != frames) {
		int willWrite = std::min(frames-written, output_frames-consumed_output_frames);
		if(willWrite <= 0) {
			refillOutputBuffer();
			continue;
		}
		std::copy(output_buffer+consumed_output_frames*output_channels, output_buffer+consumed_output_frames*output_channels+willWrite*output_channels, buffer+written*output_channels);
		written+=willWrite;
		consumed_output_frames += willWrite;
	}
}

void SampleFormatConverter::refillOutputBuffer() {
	//Either we pull and downmix, or we pull, resample, and downmix.
	//First the fast path: no resampling is needed.  This is by far the easiest.
	if(input_sr == output_sr) {
		//If the channels are also the same, we can short-circuit everything here and just write to the output.
		if(input_channels == output_channels) callback(output_buffer, input_channels);
		//Otherwise, we'll need a remix.
		else {
			callback(input_buffer, input_channels);
			::audio_io::remixAudioInterleaved(input_frames, input_channels, input_buffer, output_channels, output_buffer);
		}
	}
	//This is the alternative, slower path.
	//resample to resampler_workspace, then downmix to output_buffer.
	else {
		int required = output_frames;
		int got = 0;
		while(got < required) {
			got += resampler->write(resampler_workspace+got*input_channels, required-got);
			//We read first so that we can ensure that we're not about to build up latency.
			//But we may not have gotten enough.
			if(got < required) {
				callback(input_buffer, input_channels);
				resampler->read(input_buffer);
			}
		}
		//We either downmix or outright copy, depending if the channel counts are different.
		if(input_channels == output_channels) std::copy(resampler_workspace, resampler_workspace+output_frames*output_channels, output_buffer);
		else ::audio_io::remixAudioInterleaved(output_frames, input_channels, resampler_workspace, output_channels, output_buffer);
	}
	consumed_output_frames = 0;
}

}
}