/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/private/kernels.hpp>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

void applyMixingMatrix(int sampleCount, int inputChannels, float** inputs, int outputChannels, float** outputs, const float* mixingMatrix) {
	//loop over the matrix rows. Apply multiplicationAdditionKernel.
	for(int i= 0; i < outputChannels; i++) {
		for(int j = 0; j < inputChannels; j++) {
			multiplicationAdditionKernel(sampleCount, *(mixingMatrix+i*inputChannels+j), inputs[j], outputs[i], outputs[i]);
		}
	}
}