/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <vector>

namespace libaudioverse_implementation {

//An n-channel panner.
//very very private. This does almost no error checking.

struct AmplitudePannerEntry {
	AmplitudePannerEntry(float _angle, unsigned int _c): angle(_angle), channel(_c) {}
	float angle;
	unsigned int channel;
};

class AmplitudePanner {
	public:
	void clearMap();
	void addEntry(float angle, unsigned int channel);
	void pan(float angle, unsigned int block_size, float* input, unsigned int outputCount, float** outputs);
	void readMap(int entries, float* map);
	private:
	std::vector<AmplitudePannerEntry> channels;
};

}