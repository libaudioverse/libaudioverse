/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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