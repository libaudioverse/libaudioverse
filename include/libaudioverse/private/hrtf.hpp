/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once
#include <powercores/thread_local_variable.hpp>
#include <string>
#include <memory>
#include <kiss_fftr.h>
#include <tuple>

namespace libaudioverse_implementation {

class HrtfData {
	public:
	HrtfData();
	~HrtfData();
	//get the appropriate coefficients for one channel.  A stereo hrtf is two calls to this function.
	float computeCoefficientsMono(float elevation, float azimuth, float* out);

	//warning: writes directly to the output destination, doesn't allocate a new one.
	// Returns the delays for the left and right ear.
	std::tuple<float, float> computeCoefficientsStereo(float elevation, float azimuth, float* left, float* right);

	//load from a file.
	void loadFromFile(std::string path, unsigned int forSr);
	void loadFromDefault(unsigned int forSr);
	void loadFromBuffer(unsigned int length, char* buffer, unsigned int forSr);

	//get the hrir's length.
	int getLength();
	private:
	float* createTemporaryBuffer();
	void freeTemporaryBuffer(float* b);
	int elev_count = 0, hrir_count = 0, hrir_length = 0;
	int min_elevation = 0, max_elevation = 0;
	int *azimuth_counts = nullptr;
	int samplerate = 0;
	float ***hrirs = nullptr, **hrir_delays = nullptr;
	//used for crossfading so we don't clobber the heap.
	powercores::ThreadLocalVariable<float*> temporary_buffer1, temporary_buffer2;
};

void initializeHrtfCaches();
void shutdownHrtfCaches();

//This is threadsafe in and of itself, and will return hrtfs from a cache if it can.
//Either load from a file or our internal default.
std::shared_ptr<HrtfData> createHrtfFromString(std::string path, int forSr);
}