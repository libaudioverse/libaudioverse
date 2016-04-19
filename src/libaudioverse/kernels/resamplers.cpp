/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/macros.hpp>
#include <speex_resampler_cpp.hpp>

namespace libaudioverse_implementation {

void staticResamplerKernel(int inputSr, int outputSr, int channels, int frames, float* data, int *outLength, float** outData) {
	try {
		speex_resampler_cpp::staticResampler(inputSr, outputSr, channels, frames, data, outLength, outData);
	}
	catch(speex_resampler_cpp::MemoryAllocationError& e) {
		ERROR(Lav_ERROR_MEMORY, "Could not call speex_resampler_cpp::staticResampler.");
	}
	catch(speex_resampler_cpp::SpeexError &e) {
		ERROR(Lav_ERROR_INTERNAL, std::string("Speex error: ")+std::string(e.what()));
	}
}

}