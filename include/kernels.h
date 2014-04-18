/**These are kernels, which implement the mathematical operations required for DSP.

This is internal, and should not be used by public code.*/

void convolution_kernel(float* output, const int samplesToCalculate, float const* input, const unsigned int lengthOfImpulse, float const* impulseResponse);