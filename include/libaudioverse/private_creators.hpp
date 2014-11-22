/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <memory>

class LavObject;
class LavSimulation;
class LavHrtfData;

std::shared_ptr<LavObject> createAttenuatorObject(std::shared_ptr<LavSimulation> simulation, unsigned int numChannels);
std::shared_ptr<LavObject> createFileObject(std::shared_ptr<LavSimulation> simulation, const char* path);
std::shared_ptr<LavObject> createHardLimiterObject(std::shared_ptr<LavSimulation> simulation, unsigned int numChannels);
std::shared_ptr<LavObject> createHrtfObject(std::shared_ptr<LavSimulation> simulation, std::shared_ptr<LavHrtfData> hrtf);
std::shared_ptr<LavObject> createMixerObject(std::shared_ptr<LavSimulation> simulation, unsigned int maxParents, unsigned int inputsPerParent);
std::shared_ptr<LavObject> createSineObject(std::shared_ptr<LavSimulation> Simulation);
std::shared_ptr<LavObject> createDelayObject(std::shared_ptr<LavSimulation> simulation, unsigned int lines);
std::shared_ptr<LavObject>createAmplitudePannerObject(std::shared_ptr<LavSimulation> Simulation);
std::shared_ptr<LavObject> createCustomObject(std::shared_ptr<LavSimulation> sim, unsigned int inputs, unsigned int outputs);
std::shared_ptr<LavObject> createGraphListenerObject(std::shared_ptr<LavSimulation> sim, unsigned int channels);
std::shared_ptr<LavObject> createPullObject(std::shared_ptr<LavSimulation> sim, unsigned int inputSr, unsigned int channels);
