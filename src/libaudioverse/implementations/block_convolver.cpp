/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/implementations/convolvers.hpp>
#include <algorithm>
#include <functional>
#include <math.h>

namespace libaudioverse_implementation {

BlockConvolver::BlockConvolver(int blockSize): block_size(blockSize) {
	float defaultResponse=1;
	setResponse(1, &defaultResponse);
}

BlockConvolver::~BlockConvolver() {
	if(response) freeArray(response);
	if(history) freeArray(history);
}

void BlockConvolver::setResponse(int length, float* newResponse) {
	if(response== nullptr) {
		response=allocArray<float>(length);
	}
	else if(response_length != length) {
		freeArray(response);
		response=allocArray<float>(length);
	}
	std::copy(newResponse, newResponse+length, response);
	if(history == nullptr) {
		history = allocArray<float>(block_size+length);
	}
	else if(length !=response_length) {
		freeArray(history);
		history = allocArray<float>(block_size+length);
	}
	response_length= length;
}

void BlockConvolver::convolve(float* input, float* output) {
	//First, move the history back.
	int historyLength =response_length+block_size;
	std::copy(history+historyLength-response_length, history+historyLength, history);
	std::copy(input, input+block_size, history+historyLength-block_size);
	convolutionKernel(history, block_size, output, response_length, response);
}

void BlockConvolver::reset() {
	std::fill(history, history+block_size+response_length, 0.0f);
}

}