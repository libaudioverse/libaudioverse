/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Demonstrates 3d api.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/libaudioverse3d.h>
#include <stdio.h>
#include <thread>
#include <chrono>

#define ERRCHECK(x) do {\
if((x) != Lav_ERROR_NONE) {\
	printf(#x " errored: %i", (x));\
	return;\
}\
} while(0)\

void main(int argc, char** args) {
	if(argc != 3) {
		printf("Usage:%s <soundfile> <hrtf>", args[0]);
		return;
	}
	char *soundFile = args[1], *hrtfFile = args[2];
	LavDevice* device;
	LavObject* node, *world, *source;
	ERRCHECK(Lav_initializeLibrary());
	ERRCHECK(Lav_createDeviceForPhysicalOutput(-1, 44100, 512, 2, &device));
	ERRCHECK(Lav_createWorldObject(device, hrtfFile, &world));
	ERRCHECK(Lav_createFileObject(device, soundFile, &node));
	ERRCHECK(Lav_createSourceObject(device, world, &source));
	ERRCHECK(Lav_objectSetInput(source, 0, node, 0));
	const int resolution = 1000, length = 3000; //length in ms.
	const float width = 30.0;
	Lav_deviceSetOutputObject(device, world);
	//do a square over and over.
	while(1) {
		for(int i = 0; i < resolution; i++) {
			Lav_objectSetFloat3Property(source, Lav_3D_POSITION, width*(i/(float)resolution)-width/2, 0, -width/2);
			std::this_thread::sleep_for(std::chrono::milliseconds(length/resolution));
		}
		for(int i = 0; i < resolution; i++) {
			Lav_objectSetFloat3Property(source, Lav_3D_POSITION, width/2, 0, (width*((float)i/resolution)-width/2) );
			std::this_thread::sleep_for(std::chrono::milliseconds(length/resolution));
		}
		for(int i = 0; i < resolution; i++) {
			Lav_objectSetFloat3Property(source, Lav_3D_POSITION, -(width*(i/(float)resolution)-width/2), 0, width/2);
			std::this_thread::sleep_for(std::chrono::milliseconds(length/resolution));
		}
		for(int i = 0; i < resolution; i++) {
			Lav_objectSetFloat3Property(source, Lav_3D_POSITION, -width/2, 0, -(width*((float)i/resolution)-width/2));
			std::this_thread::sleep_for(std::chrono::milliseconds(length/resolution));
		}
	}
}
