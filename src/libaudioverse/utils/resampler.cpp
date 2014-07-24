/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <libaudioverse/private_resampler.hpp>
#include <libaudioverse/private_dspmath.hpp>
#include <libaudioverse/private_kernels.hpp>
#include <algorithm>
#include <utility>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <libaudioverse/private_dspmath.hpp>
LavResampler::LavResampler(int sourceSr, int targetSr, std::function<std::tuple<int, float*>(void)> getFromCallback): source_sr(sourceSr), target_sr(targetSr), get_from_callback(getFromCallback) {
	delta = (float)sourceSr/(float)targetSr;
}
float LavResampler::getNext() {
	if(current_pos >= std::get<0>(current_pair)-1) {
		last_sample = std::get<1>(current_pair)[std::get<0>(current_pair)];
		current_pair = get_from_callback();
		current_pos = 0;
	}
	float w1 = 1-current_offset;
	float w2 = current_offset;
	float s1, s2;
	if(current_pos == 0) {
		s1 = last_sample;
		s2 = std::get<1>(current_pair)[0];
	}
	else {
		s1 = std::get<1>(current_pair)[current_pos];
		s2 = std::get<1>(current_pair)[current_pos+1];
	}
	//update current_pos and current_offset.
	current_offset += delta;
	current_pos += (int)floorf(current_offset);
	current_offset = current_offset-floorf(current_offset);
	return w1*s1+w2*s2;
}

void LavResampler::read(int count, float* output) {
	for(int i = 0; i < count; i++) {
		output[i] = getNext();
	}
}
