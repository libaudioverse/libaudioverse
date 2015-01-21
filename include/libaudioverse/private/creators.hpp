/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <memory>

class LavNode;
class LavSimulation;
class LavHrtfData;

std::shared_ptr<LavNode> createAttenuatorNode(std::shared_ptr<LavSimulation> simulation, unsigned int numChannels);
std::shared_ptr<LavNode> createFileNode(std::shared_ptr<LavSimulation> simulation, const char* path);
std::shared_ptr<LavNode> createHardLimiterNode(std::shared_ptr<LavSimulation> simulation, unsigned int numChannels);
std::shared_ptr<LavNode> createHrtfNode(std::shared_ptr<LavSimulation> simulation, std::shared_ptr<LavHrtfData> hrtf);
std::shared_ptr<LavNode> createSineNode(std::shared_ptr<LavSimulation> Simulation);
std::shared_ptr<LavNode> createSquareNode(std::shared_ptr<LavSimulation> simulation);
std::shared_ptr<LavNode> createNoiseNode(std::shared_ptr<LavSimulation> simulation);

std::shared_ptr<LavNode> createDelayNode(std::shared_ptr<LavSimulation> simulation, unsigned int lines);
std::shared_ptr<LavNode>createAmplitudePannerNode(std::shared_ptr<LavSimulation> Simulation);
std::shared_ptr<LavNode> createCustomNode(std::shared_ptr<LavSimulation> sim, unsigned int inputs, unsigned int outputs);
std::shared_ptr<LavNode> createGraphListenerNode(std::shared_ptr<LavSimulation> sim, unsigned int channels);
std::shared_ptr<LavNode> createPullNode(std::shared_ptr<LavSimulation> sim, unsigned int inputSr, unsigned int channels);
std::shared_ptr<LavNode> createMultipannerNode(std::shared_ptr<LavSimulation> sim, std::shared_ptr<LavHrtfData> hrtf);
std::shared_ptr<LavNode> createFeedbackDelayNetworkNode(std::shared_ptr<LavSimulation> simulation, float maxDelay, int lines);
std::shared_ptr<LavNode> createMultifileNode(std::shared_ptr<LavSimulation> simulation, int channels, int maxSimultaneousFiles);

std::shared_ptr<LavNode> createIirNode(std::shared_ptr<LavSimulation> simulation, int channels);

std::shared_ptr<LavNode> createGainNode(std::shared_ptr<LavSimulation> sim);