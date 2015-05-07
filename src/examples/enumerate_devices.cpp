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
	Lav_shutdown();\
	return;\
}\
} while(0)\

void main() {
	ERRCHECK(Lav_initialize());
	unsigned int max_devices = 0;
	ERRCHECK(Lav_deviceGetCount(&max_devices));
	printf("%u devices detected.\n", max_devices);
	for(unsigned int i = 0; i < max_devices; i++) {
		printf("\n\n");
		char* name;
		ERRCHECK(Lav_deviceGetName(i, &name));
		printf("%s:\n", name);
		Lav_free(name);
		unsigned int channels = 0;
		ERRCHECK(Lav_deviceGetChannels(i, &channels));
		printf("channels: %u\n", channels);
	}
	Lav_shutdown();
}