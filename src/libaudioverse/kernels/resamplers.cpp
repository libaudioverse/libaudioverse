/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
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