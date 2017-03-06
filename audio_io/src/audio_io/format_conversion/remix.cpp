#include <audio_io/audio_io.hpp>
#include <audio_io/private/mixing_matrices.hpp>
#include <algorithm>


namespace audio_io {
namespace implementation {

//These are helpers.  The definition of the public functions is at the bottom of this file.

void upmixMonoUninterleaved(int frames, float* input, int outputChannels, float** outputs) {
	for(int i = 0; i < frames; i++) {
		for(int j = 0; j < outputChannels; j++) {
			outputs[j][i] += input[i];
		}
	}
}

void upmixMonoInterleaved(int frames, float* input, int outputChannels, float* output) {
	for(int i = 0, j = 0; i < frames; i++, j+=outputChannels) {
		for(int chan = 0; chan < outputChannels; chan++) output[j+chan]+=input[i];
	}
}

void mixUnrecognizedUninterleaved(int frames, int inputChannels, float** inputs, int outputChannels, float** outputs) {
	int neededChannels = std::min(inputChannels, outputChannels);
	for(int i = 0; i < neededChannels; i++) {
		for(int frame = 0; frame < frames; frame++) outputs[i][frame] += inputs[i][frame];
	}
}

void mixUnrecognizedInterleaved(int frames, int inputChannels, float* input, int outputChannels, float* output) {
	int neededChannels = std::min(inputChannels, outputChannels);
	for(int in = 0, out = 0; in < frames*inputChannels; in+=inputChannels, out += outputChannels) {
		for(int j = 0; j < neededChannels; j++) output[out+j] += input[in+j];
	}
}

//We do these as templates so we can get our money's worth with unrolling and simd and etc.
template<float* matrix, int inputChannels, int outputChannels>
void applyMixingMatrixUninterleaved(int frames, float** inputs, float** outputs) {
	float frame[inputChannels];
	for(int i = 0; i < frames; i++) {
		for(int j = 0; j < inputChannels; j++) frame[j] = inputs[j][i];
		for(int row = 0; row < outputChannels; row++) {
			float *curRow = matrix+row*inputChannels;
			for(int column = 0; column < inputChannels; column++) outputs[row][i] += frame[column]*curRow[column];
		}
	}
}

template<float* matrix, int inputChannels, int outputChannels>
void applyMixingMatrixInterleaved(int frames, float* input, float* output) {
	float frame[inputChannels];
	for(int in = 0, out = 0; in < inputChannels*frames; in+=inputChannels, out += outputChannels) {
		std::copy(input+in, input+in+inputChannels, frame);
		for(int row = 0; row < outputChannels; row++) {
			float* curRow = matrix+row*inputChannels;
			for(int column = 0; column < inputChannels; column++) output[out+row] += curRow[column]*frame[column];
		}
	}
}

} //close off the implementation namespace.

//Everything below here is public.
using namespace implementation;

void remixAudioInterleaved(int frames, int inputChannels, float* input, int outputChannels, float* output, bool zeroFirst) {
	if(zeroFirst) std::fill(output, output+frames*outputChannels, 0.0f);
	if(inputChannels == 1 && outputChannels == 2) applyMixingMatrixInterleaved<mixing_matrix_1_2, 1, 2>(frames, input, output);
	else if(inputChannels == 1 && outputChannels == 4) applyMixingMatrixInterleaved<mixing_matrix_1_4, 1, 4>(frames, input, output);
	else if(inputChannels == 1 && outputChannels == 6) applyMixingMatrixInterleaved<mixing_matrix_1_6, 1, 6>(frames, input, output);
	else if(inputChannels == 1 && outputChannels == 8) applyMixingMatrixInterleaved<mixing_matrix_1_8, 1, 8>(frames, input, output);
	else if(inputChannels == 1) upmixMonoInterleaved(frames, input, outputChannels, output);
	else if(inputChannels == 2 && outputChannels == 1) applyMixingMatrixInterleaved<mixing_matrix_2_1, 2, 1>(frames, input, output);
	else if(inputChannels == 2 && outputChannels == 4) applyMixingMatrixInterleaved<mixing_matrix_2_4, 2, 4>(frames, input, output);
	else if(inputChannels == 2 && outputChannels == 6) applyMixingMatrixInterleaved<mixing_matrix_2_6, 2, 6>(frames, input, output);
	else if(inputChannels == 2 && outputChannels == 8) applyMixingMatrixInterleaved<mixing_matrix_2_8, 2, 8>(frames, input, output);
	else if(inputChannels == 4 && outputChannels == 1) applyMixingMatrixInterleaved<mixing_matrix_4_1, 4, 1>(frames, input, output);
	else if(inputChannels == 4 && outputChannels == 2) applyMixingMatrixInterleaved<mixing_matrix_4_2, 4, 2>(frames, input, output);
	else if(inputChannels == 4 && outputChannels == 6) applyMixingMatrixInterleaved<mixing_matrix_4_6, 4, 6>(frames, input, output);
	else if(inputChannels == 4 && outputChannels == 8) applyMixingMatrixInterleaved<mixing_matrix_4_8, 4, 8>(frames, input, output);
	else if(inputChannels == 6 && outputChannels == 1) applyMixingMatrixInterleaved<mixing_matrix_6_1, 6, 1>(frames, input, output);
	else if(inputChannels == 6 && outputChannels == 2) applyMixingMatrixInterleaved<mixing_matrix_6_2, 6, 2>(frames, input, output);
	else if(inputChannels == 6 && outputChannels == 4) applyMixingMatrixInterleaved<mixing_matrix_6_4, 6, 4>(frames, input, output);
	else if(inputChannels == 6 && outputChannels == 8) applyMixingMatrixInterleaved<mixing_matrix_6_8, 6, 8>(frames, input, output);
	else if(inputChannels == 8 && outputChannels == 1) applyMixingMatrixInterleaved<mixing_matrix_8_1, 8, 1>(frames, input, output);
	else if(inputChannels == 8 && outputChannels == 2) applyMixingMatrixInterleaved<mixing_matrix_8_2,  8, 2>(frames, input, output);
	else if(inputChannels == 8 && outputChannels == 4) applyMixingMatrixInterleaved<mixing_matrix_8_4, 8, 4>(frames, input, output);
	else if(inputChannels == 8 && outputChannels == 6) applyMixingMatrixInterleaved<mixing_matrix_8_6, 8, 6>(frames, input, output);
	else mixUnrecognizedInterleaved(frames, inputChannels, input, outputChannels, output);
}

void remixAudioUninterleaved(int frames, int inputChannels, float** inputs, int outputChannels, float** outputs, bool zeroFirst) {
	if(zeroFirst) for(int i = 0; i < outputChannels; i++) std::fill(outputs[i], outputs[i]+frames, 0.0f);
	if(inputChannels == 1 && outputChannels == 2) applyMixingMatrixUninterleaved<mixing_matrix_1_2, 1, 2>(frames, inputs, outputs);
	else if(inputChannels == 1 && outputChannels == 4) applyMixingMatrixUninterleaved<mixing_matrix_1_4, 1, 4>(frames, inputs, outputs);
	else if(inputChannels == 1 && outputChannels == 6) applyMixingMatrixUninterleaved<mixing_matrix_1_6, 1, 6>(frames, inputs, outputs);
	else if(inputChannels == 1 && outputChannels == 8) applyMixingMatrixUninterleaved<mixing_matrix_1_8, 1, 8>(frames, inputs, outputs);
	else if(inputChannels == 1) upmixMonoUninterleaved(frames, inputs[0], outputChannels, outputs);
	else if(inputChannels == 2 && outputChannels == 1) applyMixingMatrixUninterleaved<mixing_matrix_2_1, 2, 1>(frames, inputs, outputs);
	else if(inputChannels == 2 && outputChannels == 4) applyMixingMatrixUninterleaved<mixing_matrix_2_4, 2, 4>(frames, inputs, outputs);
	else if(inputChannels == 2 && outputChannels == 6) applyMixingMatrixUninterleaved<mixing_matrix_2_6, 2, 6>(frames, inputs, outputs);
	else if(inputChannels == 2 && outputChannels == 8) applyMixingMatrixUninterleaved<mixing_matrix_2_8, 2, 8>(frames, inputs, outputs);
	else if(inputChannels == 4 && outputChannels == 1) applyMixingMatrixUninterleaved<mixing_matrix_4_1, 4, 1>(frames, inputs, outputs);
	else if(inputChannels == 4 && outputChannels == 2) applyMixingMatrixUninterleaved<mixing_matrix_4_2, 4, 2>(frames, inputs, outputs);
	else if(inputChannels == 4 && outputChannels == 6) applyMixingMatrixUninterleaved<mixing_matrix_4_6, 4, 6>(frames, inputs, outputs);
	else if(inputChannels == 4 && outputChannels == 8) applyMixingMatrixUninterleaved<mixing_matrix_4_8, 4, 8>(frames, inputs, outputs);
	else if(inputChannels == 6 && outputChannels == 1) applyMixingMatrixUninterleaved<mixing_matrix_6_1, 6, 1>(frames, inputs, outputs);
	else if(inputChannels == 6 && outputChannels == 2) applyMixingMatrixUninterleaved<mixing_matrix_6_2, 6, 2>(frames, inputs, outputs);
	else if(inputChannels == 6 && outputChannels == 4) applyMixingMatrixUninterleaved<mixing_matrix_6_4, 6, 4>(frames, inputs, outputs);
	else if(inputChannels == 6 && outputChannels == 8) applyMixingMatrixUninterleaved<mixing_matrix_6_8, 6, 8>(frames, inputs, outputs);
	else if(inputChannels == 8 && outputChannels == 1) applyMixingMatrixUninterleaved<mixing_matrix_8_1, 8, 1>(frames, inputs, outputs);
	else if(inputChannels == 8 && outputChannels == 2) applyMixingMatrixUninterleaved<mixing_matrix_8_2,  8, 2>(frames, inputs, outputs);
	else if(inputChannels == 8 && outputChannels == 4) applyMixingMatrixUninterleaved<mixing_matrix_8_4, 8, 4>(frames, inputs, outputs);
	else if(inputChannels == 8 && outputChannels == 6) applyMixingMatrixUninterleaved<mixing_matrix_8_6, 8, 6>(frames, inputs, outputs);
	else mixUnrecognizedUninterleaved(frames, inputChannels, inputs, outputChannels, outputs);
}

}