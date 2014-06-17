/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <functional> //we have to use an std::function for the preprocessing hook.  There's no good way around it because worlds need to use capturing lambdas.

class LavObject;

//audio devices.
//the specific functionality of an audio device needs to be hidden behind the void* data parameter, but the three function pointers *must* be filled out.
//furthermore, mutex *must* be set to something and block_size must be greater than 0.
//The above assumptions are made throughout the entire library.
class LavDevice {
	public:
	virtual ~LavDevice();
	virtual LavError getBlock(float* out);
	virtual LavError start();
	virtual LavError stop();
	std::function<void(void)> preprocessing_hook;
	unsigned int block_size, channels, mixahead;
	float sr;
	void* mutex, *device_specific_data;
	LavObject** objects;
	unsigned object_count, max_object_count;
	LavObject* output_object;
};

//initialize the audio backend.
LavError initializeAudioBackend();
//any null callback gets replaced with a default implementation that "does something sensible".

LavError deviceAssociateObject(LavDevice* device, LavObject* object);
LavDevice* createPortaudioDevice(unsigned int sr, unsigned int channels, unsigned int bufferSize, unsigned int mixahead);
