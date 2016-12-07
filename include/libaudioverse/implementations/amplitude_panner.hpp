/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once
#include <vector>

namespace libaudioverse_implementation {

//An n-channel panner.
//very very private. This does almost no error checking.

struct AmplitudePannerEntry {
	AmplitudePannerEntry(float _angle, unsigned int _c): angle(_angle), channel(_c) {}
	float angle;
	int channel;
};

class AmplitudePanner {
	public:
	AmplitudePanner(int _block_size, float _sr);
	void clearMap();
	void addEntry(float angle, int channel);
	void pan(float* input, float** outputs);
	void readMap(int entries, float* map);
	float getAzimuth();
	void setAzimuth(float a);
	float getElevation();
	void setElevation(float e);
	private:
	std::vector<AmplitudePannerEntry> channels;
	float azimuth = 0.0f, elevation = 0.0f;
	float sr;
	int block_size;
};

}