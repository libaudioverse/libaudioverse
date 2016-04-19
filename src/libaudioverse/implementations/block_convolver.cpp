/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
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