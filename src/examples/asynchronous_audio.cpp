/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Demonstrates asynchronous audio output.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <thread>
#include <chrono>
#include <stdio.h>

#define ERRCHECK(x) do {\
if((x) != Lav_ERROR_NONE) {\
	printf(#x " errored: %i", (x));\
	return;\
}\
} while(0)\

void main() {
		LavDevice* device;
	LavObject* node;
	ERRCHECK(Lav_initializeLibrary());
	ERRCHECK(Lav_createDefaultAudioOutputDevice(&device));
	ERRCHECK(Lav_createSineObject(device, &node));
	ERRCHECK(Lav_deviceSetOutputObject(device, node));
//	ERRCHECK(Lav_objectSetFloatProperty(node, Lav_SINE_FREQUENCY, 0));
	for(unsigned int i = 0; i < 1000; i++) {
//		ERRCHECK(Lav_objectSetFloatProperty(node, Lav_SINE_FREQUENCY, (float)i));
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
}
