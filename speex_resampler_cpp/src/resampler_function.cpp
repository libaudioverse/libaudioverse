#include <speex_resampler_cpp.hpp>
#include <algorithm>
#include <math.h>
#include <inttypes.h>
#include "speex_resampler.h"

namespace speex_resampler_cpp {

void staticResampler(int inputSr, int outputSr, int channels, int frames, float* data, int *outLength, float** outData) {
	if(inputSr== outputSr) { //copy
		float* o = (float*)calloc(channels*frames, sizeof(float));
		if(o == nullptr) throw MemoryAllocationError();
		std::copy(data, data+channels*frames, o);
		*outLength = frames;
		*outData = o;
		return;
	}
	int err;
	auto resampler=speex_resampler_init(channels, inputSr, outputSr, 10, &err);
	if(resampler == nullptr || err == RESAMPLER_ERR_ALLOC_FAILED) throw MemoryAllocationError();
	else if(err != RESAMPLER_ERR_SUCCESS) throw SpeexError(err);
	//The 200 makes sure that we grab all of it.
	//It is not inconceivable that we might resample by a huge rate.
	//And speex doesn't let us estimate output.
	//Instead of reallocating, we waste a small amount of ram here.
	//Note that the first expression can actually overflow a 32-bit integer.
	int size=(int)((int64_t)frames*channels*outputSr/inputSr+channels*200);
	float* o = (float*)calloc(size, sizeof(float));
	unsigned int written = 0, consumed = 0;
	*outLength= 0;
	int remaining_in =frames, remaining_out = size/channels+200;
	float* tempData=data, *tempO=o;
	while(remaining_in > 0 && remaining_out > 0) {
		consumed=std::min<int>(1024, remaining_in);
		written=std::min(1024, remaining_out);
		speex_resampler_process_interleaved_float(resampler, tempData, &consumed, tempO, &written);
		remaining_in -= consumed;
		remaining_out -= written;
		*outLength +=written;
		tempData += consumed*channels;
		tempO += written*channels;
	}
	*outData = o;
	speex_resampler_destroy(resampler);
}

}