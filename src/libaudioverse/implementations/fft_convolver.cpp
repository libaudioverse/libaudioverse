/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/implementations/convolvers.hpp>
#include <algorithm>
#include <functional>
#include <math.h>
#include <kiss_fftr.h>

namespace libaudioverse_implementation {

FftConvolver::FftConvolver(int blockSize): block_size(blockSize) {
	float defaultResponse=1;
	setResponse(1, &defaultResponse);
}

FftConvolver::~FftConvolver() {
	if(workspace) freeArray(workspace);
	if(tail) freeArray(tail);
	if(fft) kiss_fftr_free(fft);
	if(ifft) kiss_fftr_free(ifft);
}

void FftConvolver::setResponse(int length, float* newResponse) {
	int neededLength= kiss_fftr_next_fast_size_real	(block_size+length);
	int newTailSize=neededLength-block_size;
	if(neededLength !=fft_size || tail_size !=newTailSize) {
		if(workspace) freeArray(workspace);
		workspace=allocArray<float>(neededLength);
		if(tail) freeArray(tail);
		tail=allocArray<float>(newTailSize);
		fft_size=neededLength/2+1;
		workspace_size=neededLength;
		tail_size=newTailSize;
		if(fft) kiss_fftr_free(fft);
		fft = kiss_fftr_alloc(workspace_size, 0, nullptr, nullptr);
		if(ifft) kiss_fftr_free(ifft);
		ifft=kiss_fftr_alloc(workspace_size, 1, nullptr, nullptr);
		if(response_fft) freeArray(response_fft);
		response_fft=allocArray<kiss_fft_cpx>(fft_size);
		if(block_fft) freeArray(block_fft);
		block_fft=allocArray<kiss_fft_cpx>(fft_size);
	}
	memset(workspace, 0, sizeof(float)*workspace_size);
	//Store the fft of the response.
	std::copy(newResponse, newResponse+length, workspace);
	kiss_fftr(fft, workspace, response_fft);
}

void FftConvolver::convolve(float* input, float* output) {
	convolveFft(getFft(input), output);
}

int FftConvolver::getFftSize() {
	return workspace_size;
}

kiss_fft_cpx *FftConvolver::getFft(float* input) {
	//We reuse workspace, so have to zero the tail part of it.
	std::fill(workspace+block_size, workspace+workspace_size, 0.0);
	//Copy input to the workspace, and take its fft.
	std::copy(input, input+block_size, workspace);
	kiss_fftr(fft, workspace, block_fft);
	return block_fft;
}

void FftConvolver::convolveFft(kiss_fft_cpx *fft, float* output) {
	//Do a complex multiply.
	//Note that the first line is subtraction because of the i^2.
	for(int i=0; i < fft_size; i++) {
		kiss_fft_cpx tmp;
		tmp.r = fft[i].r*response_fft[i].r-fft[i].i*response_fft[i].i;
		tmp.i = fft[i].r*response_fft[i].i+fft[i].i*response_fft[i].r;
		block_fft[i] = tmp;
	}
	kiss_fftri(ifft, block_fft, workspace);
	//Add the tail over the block.
	additionKernel(tail_size, tail, workspace, workspace);
	//Downscale the first part, our output.
	//Also copy to the output at the same time.
	scalarMultiplicationKernel(block_size, 1.0/workspace_size, workspace, output);
	//Copy out the tail.
		std::copy(workspace+block_size, workspace+workspace_size, tail);
}

void FftConvolver::reset() {
	std::fill(workspace, workspace+workspace_size, 0.0f);
	std::fill(tail, tail+tail_size, 0.0f);
}

}