/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "private_structs.hpp"

//audio devices.
//the specific functionality of an audio device needs to be hidden behind the void* data parameter, but the three function pointers *must* be filled out.
//furthermore, mutex *must* be set to something and block_size must be greater than 0.
//The above assumptions are made throughout the entire library.
struct LavDevice {
	LavError (*get_block)(LavDevice* device, float* destination);
	LavError (*start)(LavDevice* device);
	LavError (*stop)(LavDevice *device);
	LavError (*kill)(LavDevice* device);
	//this one is optional.  Intended for simulations and the like that want a chance to do stuff every block.
	void (*preprocessing_hook)(LavDevice* device, void* argument);
	void* preprocessing_hook_argument;
	unsigned int block_size, channels, mixahead;
	float sr;
	void* mutex, *device_specific_data;
	struct LavObject** objects;
	unsigned object_count, max_object_count;
	struct LavObject* output_object;
};

//initialize the audio backend.
LavError initializeAudioBackend();
//any null callback gets replaced with a default implementation that "does something sensible".
LavError createGenericDevice(
	unsigned int blockSize,
	unsigned int channels,
	unsigned int sr,
	LavError (*getBlock)(LavDevice* device, float* destination),
	LavError (*start)(LavDevice* device),
	LavError (*stop)(LavDevice* device),
	LavError (*kill)(LavDevice* device),
	LavDevice** destination);

LavError deviceAssociateObject(LavDevice* device, LavObject* object);
LavError portaudioDeviceConfigurer(LavDevice* device);
