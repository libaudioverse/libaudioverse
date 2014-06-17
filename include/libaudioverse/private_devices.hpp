/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <functional> //we have to use an std::function for the preprocessing hook.  There's no good way around it because worlds need to use capturing lambdas.
#include <set>
#include "libaudioverse.h"

class LavObject;

class LavDevice {
	public:
	LavDevice(unsigned int sr, unsigned int channels, unsigned int blockSize, unsigned int mixahead);
	virtual ~LavDevice() {}
	virtual LavError getBlock(float* out);
	virtual LavError start();
	virtual LavError stop();
	virtual LavError associateObject(LavObject* obj);
	virtual LavError setOutputObject(LavObject* obj);
	std::function<void(void)> preprocessing_hook;
	unsigned int block_size, channels, mixahead;
	float sr;
	void* mutex;
	std::set<LavObject*> objects;
	LavObject* output_object;
};

//initialize the audio backend.
LavError initializeAudioBackend();

LavDevice* createPortaudioDevice(unsigned int sr, unsigned int channels, unsigned int bufferSize, unsigned int mixahead);
