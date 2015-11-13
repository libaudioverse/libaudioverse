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
	LavHandle simulation;
	LavHandle node;
	ERRCHECK(Lav_initialize());
	ERRCHECK(Lav_createSimulation(44100, 8192, &simulation));
	ERRCHECK(Lav_simulationSetOutputDevice(simulation, -1, 2, 0.0, 0.1, 0.2));
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
}
