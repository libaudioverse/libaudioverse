/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <powercores/thread_local_variable.hpp>
#include <string>
#include <memory>
#include <kiss_fftr.h>

namespace libaudioverse_implementation {

class HrtfData {
	public:
	HrtfData();
	~HrtfData();
	//get the appropriate coefficients for one channel.  A stereo hrtf is two calls to this function.
	void computeCoefficientsMono(float elevation, float azimuth, float* out);

	//warning: writes directly to the output destination, doesn't allocate a new one.
	void computeCoefficientsStereo(float elevation, float azimuth, float* left, float* right);

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
	float ***hrirs = nullptr;
	//used for crossfading so we don't clobber the heap.
	powercores::ThreadLocalVariable<float*> temporary_buffer1, temporary_buffer2;
};

void initializeHrtfCaches();
void shutdownHrtfCaches();

//This is threadsafe in and of itself, and will return hrtfs from a cache if it can.
//Either load from a file or our internal default.
std::shared_ptr<HrtfData> createHrtfFromString(std::string path, int forSr);
}