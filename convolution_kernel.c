/**
This is a convolution kernel.  It performs convolution, but you probably figured that out.

More usefully, it takes pointers to an input and output buffer, as well as how many samples are to be calculated.

It should be noted that the input buffer is special.  The purpose of this kernel is merely to convolve a subsequence of a larger sequence with an impulse response.

Samples are expected to arrive left to right.  Let L be the length of the impulse response andd n the number of samples to be calculated.  The length of this buffer is then:
l+n
And should be filled such that the first l samples are the "history", that is samples that were processed last time.  Initially, this should be zero.
The kernel will start at the lth sample, and produce n samples.

It is assumed that the output buffer is zeroed.
*/

void convolution_kernel(float* output, const int samplesToCalculate, float const* input, const unsigned int lengthOfImpulse, float const* impulseResponse) {
	int index;
	#pragma omp parallel for if(samplesToCalculate>1024)
	for(index = 0; index < samplesToCalculate; ++index) {
		for(unsigned int impulseResponseIndex = 0; impulseResponseIndex < lengthOfImpulse; impulseResponseIndex++) {
			unsigned int sample = index + lengthOfImpulse;
			output[index] += impulseResponse[impulseResponseIndex]*input[sample-impulseResponseIndex-1]; //if not -1, we're including the current sample.
		}
	}
}
