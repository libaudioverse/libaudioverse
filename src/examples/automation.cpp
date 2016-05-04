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
	LavHandle simulation;
	LavHandle node;
	ERRCHECK(Lav_initialize());
	ERRCHECK(Lav_createSimulation(44100, 8192, &simulation));
	ERRCHECK(Lav_simulationSetOutputDevice(simulation, "default", 2));
	ERRCHECK(Lav_createSineNode(simulation, &node));
	ERRCHECK(Lav_nodeSetFloatProperty(node, Lav_OSCILLATOR_FREQUENCY, 0));
	ERRCHECK(Lav_nodeConnectSimulation(node, 0));
	ERRCHECK(Lav_automationLinearRampToValue(node, Lav_NODE_MUL, 8.0, 0.05));
	ERRCHECK(Lav_automationLinearRampToValue(node, Lav_OSCILLATOR_FREQUENCY, 2.0, 500.0));
	ERRCHECK(Lav_automationLinearRampToValue(node, Lav_OSCILLATOR_FREQUENCY, 4.0, 300.0));
	ERRCHECK(Lav_automationLinearRampToValue(node, Lav_OSCILLATOR_FREQUENCY, 6.0, 500.0));
	ERRCHECK(Lav_automationLinearRampToValue(node, Lav_OSCILLATOR_FREQUENCY, 8.0, 100.0));
	std::this_thread::sleep_for(std::chrono::milliseconds(10000));
	Lav_shutdown();
	return 0;
}
