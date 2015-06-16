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
	if(response_workspace) freeArray(response_workspace);
	if(block_workspace) freeArray(block_workspace);
	if(tail) freeArray(tail);
}

void FftConvolver::setResponse(int length, float* newResponse) {
	int neededLength=(block_size+length)+(block_size+length)%2; //next multiple of 2.
	if(neededLength !=fft_size || tail_size !=length) {
		if(block_workspace) freeArray(block_workspace);
		block_workspace=allocArray<float>(neededLength);
		if(response_workspace) freeArray(response_workspace);
		response_workspace=allocArray<float>(neededLength);
		if(tail) freeArray(tail);
		tail=allocArray<float>(length);
		fft_size=neededLength/2+1;
		workspace_size=neededLength;
		tail_size=length;
		if(fft) kiss_fftr_free(fft);
		fft = kiss_fftr_alloc(fft_size, 0, nullptr, nullptr);
		if(ifft) kiss_fftr_free(ifft);
		ifft=kiss_fftr_alloc(fft_size, 1, nullptr, nullptr);
		if(response_fft) freeArray(response_fft);
		response_fft=allocArray<kiss_fft_cpx>(fft_size);
		if(block_fft) freeArray(block_fft);
		block_fft=allocArray<kiss_fft_cpx>(fft_size);
	}
	//First, zero everything.
	memset(response_workspace, 0, sizeof(float)*fft_size);
	memset(block_workspace, 0, sizeof(float)*fft_size);
	memset(tail, 0, sizeof(float)*tail_size);
	//Store the fft of the response.
	std::copy(newResponse, newResponse+length, response_workspace);
	kiss_fftr(fft, response_workspace, response_fft);
	/*Explanation:
	Kissfft has really poor documentation.
	Nfft is the number of data points we are taking the fft of.
	Taking the real fft scales all magnitudes by nfft, and taking the inverse scales all magnitudes by 1/2, source:
	http://stackoverflow.com/questions/5628056/kissfft-scaling
	There are two ways to deal with this.
	We can multiply by 1/nfft after convolution.  Or we can bake the scaling in here.
	*/
	for(int i = 0; i < fft_size; i++) {
		response_fft[i].r /= workspace_size;
		response_fft[i].i /=workspace_size;
	}
}

void FftConvolver::convolve(float* input, float* output) {
	//We reuse block_workspace, so have to zero it.
	memset(block_workspace, 0, workspace_size*sizeof(float));
	//Copy input to the block_workspace, and take its fft.
	std::copy(input, input+block_size, block_workspace);
	kiss_fftr(fft, block_workspace, block_fft);
	//Copy first block_size elements of tail to the output.
	memset(output, 0, block_size*sizeof(float));
	std::copy(tail, tail+std::min(block_size, tail_size), output);
	//Roll back the tail.
	std::copy(tail+std::min(block_size, tail_size), tail+tail_size, tail);
	std::fill(tail+std::min(block_size, tail_size), tail+tail_size, 0.0f);
	//Do a complex multiply.
	for(int i=0; i < fft_size; i++) {
		block_fft[i].r=block_fft[i].r*response_fft[i].r+block_fft[i].i*response_fft[i].i;
		block_fft[i].i = block_fft[i].r*response_fft[i].i+block_fft[i].i*response_fft[i].r;
	}
	kiss_fftri(ifft, block_fft, block_workspace);
	//Copy the block out, adding over the tail.
	additionKernel(block_size, block_workspace, output, output);
	//Add the new tail over the old one.
	additionKernel(tail_size, block_workspace+block_size, tail, tail);
}

}