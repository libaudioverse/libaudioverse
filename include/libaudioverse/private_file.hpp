/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <sndfile.h>
#include "libaudioverse.h"
#include <memory>

/**A completely stand-alone wrapper around Libsndfile for file reading: implements both a  streaming and non-streaming interface.

This can theoreticaly handle any type of file, including codec decoding, but uses Libsndfile for the moment.*/
class LavFileReader: std::enable_shared_from_this<LavFileReader>  {
	public:
	LavFileReader(): info() {} //vc++ crashes if we try to do this the c++11 way.
	~LavFileReader();
	void open(const char* path);
	void close();
	float getSr();
	unsigned int getChannelCount();
	unsigned int getFrameCount();
	unsigned int getSampleCount();
	unsigned int readAll(float* buffer);
	unsigned int read(unsigned int frames, float* buffer);
	protected:
	SNDFILE* handle = nullptr;
	SF_INFO info;
};
