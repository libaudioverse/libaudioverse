/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
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