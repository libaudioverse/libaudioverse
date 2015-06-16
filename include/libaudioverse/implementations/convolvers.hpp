/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <kiss_fftr.h>

namespace libaudioverse_implementation {

class BlockConvolver {
	public:
	BlockConvolver(int blockSize);
	~BlockConvolver();
	//Error if length<1.
	//This is not checked.
	//Note: this function zeros the history if the new response has a different length.
	void setResponse(int length, float* newResponse);
	void convolve(float* input, float* output);
	private:
	float* response = nullptr, *history = nullptr;
	int block_size = 0, response_length = 0;
};

class FftConvolver {
	public:
	FftConvolver(int blockSize);
	~FftConvolver();
	void setResponse(int length, float* response);
	void convolve(float* input, float* output);
	private:
	int block_size, fft_size;
	float* response_workspace = nullptr, *block_workspace = nullptr, *tail = nullptr;
	kiss_fft_cfg fft = nullptr, ifft = nullptr;
};

}