#include <speex_resampler_cpp.hpp>
#include <memory>
#include <algorithm>
#include <utility>
#include <list>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "speex_resampler.h"

namespace speex_resampler_cpp {

class ResamplerImplementation: public Resampler {
	public:
	ResamplerImplementation(int inputFrameCount, int inputChannels, int inputSr, int outputSr);
	~ResamplerImplementation();
	//Write some data from the resampler to an output array.
	//returns frames written, not samples.
	int write(float* dest, int maxFrameCount) override;
	//Put some data into the resampler.
	//this copies, the buffer can be reused.
	void read(float* source) override;
	//note the estimate: this is not necessarily sample-accurate.
	int estimateAvailableFrames() override;
	private:
	float delta = 0.0f;
	std::list<float*> queue, done_queue;
	int offset=0, input_frame_count, input_channels, input_sr, output_sr;
	SpeexResamplerState* spx_resampler= nullptr;
	int spx_error= 0;
};

ResamplerImplementation::ResamplerImplementation(int inputFrameCount, int inputChannels, int inputSr, int outputSr): input_frame_count(inputFrameCount), input_channels(inputChannels), input_sr(inputSr), output_sr(outputSr) {
	delta = (float)inputSr/(float)outputSr;
	spx_resampler = speex_resampler_init(inputChannels, inputSr, outputSr, 1, &spx_error);
	if(spx_error == RESAMPLER_ERR_ALLOC_FAILED) throw MemoryAllocationError();
	else if(spx_error != RESAMPLER_ERR_SUCCESS) throw SpeexError(spx_error);
	else if(spx_resampler == nullptr) throw MemoryAllocationError(); //Yay! Defensive Programming!
}

std::shared_ptr<Resampler> createResampler(int inputFrameCount, int inputChannels, int inputSr, int outputSr) {
	return std::make_shared<ResamplerImplementation>(inputFrameCount, inputChannels, inputSr, outputSr);
}

ResamplerImplementation::~ResamplerImplementation() {
	speex_resampler_destroy(spx_resampler);
}

void ResamplerImplementation::read(float* source) {
	float* buff;
	if(done_queue.empty()) {
		buff = new float[input_frame_count*input_channels];
	}
	else {
		buff = done_queue.front();
		done_queue.pop_front();
	}
	std::copy(source, source+input_channels*input_frame_count, buff);
	queue.push_back(buff);
}

int ResamplerImplementation::write(float* dest, int maxFrameCount) {
	int count = 0;
	float* buff;
	while(count < maxFrameCount && queue.empty() == false) {
		buff=queue.front();
		spx_uint32_t remainingInputFrames = input_frame_count-offset/input_channels;
		spx_uint32_t remainingOutputFrames = maxFrameCount-count;
		speex_resampler_process_interleaved_float(spx_resampler, buff+offset, &remainingInputFrames, dest, &remainingOutputFrames);
		//unfortunately, speex uses the lengths as out parameters.
		count +=remainingOutputFrames;
		offset += remainingInputFrames*input_channels;
		dest += remainingOutputFrames*input_channels;
		if(offset == input_frame_count*input_channels) {
			done_queue.push_back(queue.front());
			queue.pop_front();
			offset = 0;
		}
	}
	//Unfortunately, the speex resampler clips. We therefore attenuate by a very small amount.  This number was determined experimentaly.
	dest -=count*input_channels;
	for(int i=0; i < count*input_channels; i++) dest[i]*=0.94;
	return count;
}

int ResamplerImplementation::estimateAvailableFrames() {
	float delta_rec = 1.0/delta;
	return queue.size()*delta_rec*input_frame_count;
}

}