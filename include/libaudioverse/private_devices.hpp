/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "private_structs.hpp"
#ifdef __cplusplus
extern "C" {
#endif

//initialize the audio backend.
Lav_PUBLIC_FUNCTION LavError initializeAudioBackend();
//any null callback gets replaced with a default implementation that "does something sensible".
Lav_PUBLIC_FUNCTION LavError createGenericDevice(
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
#ifdef __cplusplus
}
#endif