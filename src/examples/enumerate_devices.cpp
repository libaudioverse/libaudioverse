/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */

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
	return 1;\
}\
} while(0)\

int main() {
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
	return 0;
}