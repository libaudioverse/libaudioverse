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
	LavHandle server;
	LavHandle node;
	ERRCHECK(Lav_initialize());
	ERRCHECK(Lav_createServer(44100, 8192, &server));
	ERRCHECK(Lav_serverSetOutputDevice(server, "default", 2, 2));
	ERRCHECK(Lav_createSineNode(server, &node));
	ERRCHECK(Lav_nodeSetFloatProperty(node, Lav_OSCILLATOR_FREQUENCY, 0));
	ERRCHECK(Lav_nodeConnectServer(node, 0));
	ERRCHECK(Lav_automationLinearRampToValue(node, Lav_NODE_MUL, 8.0, 0.05));
	ERRCHECK(Lav_automationLinearRampToValue(node, Lav_OSCILLATOR_FREQUENCY, 2.0, 500.0));
	ERRCHECK(Lav_automationLinearRampToValue(node, Lav_OSCILLATOR_FREQUENCY, 4.0, 300.0));
	ERRCHECK(Lav_automationLinearRampToValue(node, Lav_OSCILLATOR_FREQUENCY, 6.0, 500.0));
	ERRCHECK(Lav_automationLinearRampToValue(node, Lav_OSCILLATOR_FREQUENCY, 8.0, 100.0));
	std::this_thread::sleep_for(std::chrono::milliseconds(10000));
	Lav_shutdown();
	return 0;
}
