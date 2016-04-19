/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/

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