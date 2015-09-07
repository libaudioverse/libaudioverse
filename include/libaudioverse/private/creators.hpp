/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <memory>

namespace libaudioverse_implementation {

class Node;
class Simulation;
class HrtfData;
class EnvironmentNode;

//normal nodes in order by the file they are in, and the position in that file.
std::shared_ptr<Node>createAmplitudePannerNode(std::shared_ptr<Simulation> Simulation);
std::shared_ptr<Node> createBiquadNode(std::shared_ptr<Simulation> sim, unsigned int channels);
std::shared_ptr<Node> createBufferNode(std::shared_ptr<Simulation> simulation);
std::shared_ptr<Node> createBufferTimelineNode(std::shared_ptr<Simulation> simulation, int channels);
std::shared_ptr<Node> createChannelMergerNode(std::shared_ptr<Simulation> simulation, int channels);
std::shared_ptr<Node> createChannelSplitterNode(std::shared_ptr<Simulation> simulation, int channels);
std::shared_ptr<Node> createConvolverNode(std::shared_ptr<Simulation> simulation, int channels);
std::shared_ptr<Node> createCrossfadingDelayNode(std::shared_ptr<Simulation> simulation, int channels);
std::shared_ptr<Node> createCustomNode(std::shared_ptr<Simulation> sim, unsigned int inputs, unsigned int outputs);
std::shared_ptr<Node> createDoppleringDelayNode(std::shared_ptr<Simulation> simulation, float maxDelay, int channels);
std::shared_ptr<Node> createFeedbackDelayNetworkNode(std::shared_ptr<Simulation> simulation, float maxDelay, int lines);
std::shared_ptr<Node> createFftConvolverNode(std::shared_ptr<Simulation> simulation, int channels);
std::shared_ptr<Node> createFilteredDelayNode(std::shared_ptr<Simulation> simulation, float maxDelay, unsigned int channels);
std::shared_ptr<Node> createGainNode(std::shared_ptr<Simulation> sim);
std::shared_ptr<Node> createGraphListenerNode(std::shared_ptr<Simulation> sim, unsigned int channels);
std::shared_ptr<Node> createHardLimiterNode(std::shared_ptr<Simulation> simulation, unsigned int numChannels);
std::shared_ptr<Node> createHrtfNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<HrtfData> hrtf);
std::shared_ptr<Node> createIirNode(std::shared_ptr<Simulation> simulation, int channels);
std::shared_ptr<Node> createLateReflectionsNode(std::shared_ptr<Simulation> simulation);
std::shared_ptr<Node> createMultipannerNode(std::shared_ptr<Simulation> sim, std::shared_ptr<HrtfData> hrtf);
std::shared_ptr<Node> createNoiseNode(std::shared_ptr<Simulation> simulation);
std::shared_ptr<Node> createPannerBankNode(std::shared_ptr<Simulation> simulation, int pannerCount, std::shared_ptr<HrtfData> hrtf);
std::shared_ptr<Node> createPullNode(std::shared_ptr<Simulation> sim, unsigned int inputSr, unsigned int channels);
std::shared_ptr<Node> createPushNode(std::shared_ptr<Simulation> sim, unsigned int inputSr, unsigned int channels);
std::shared_ptr<Node> createRecorderNode(std::shared_ptr<Simulation> simulation, int channels);
std::shared_ptr<Node> createRingmodNode(std::shared_ptr<Simulation> sim);
std::shared_ptr<Node> createSineNode(std::shared_ptr<Simulation> Simulation);
std::shared_ptr<Node> createSquareNode(std::shared_ptr<Simulation> simulation);
std::shared_ptr<Node> createThreeBandEqNode(std::shared_ptr<Simulation> simulation, int channels);


//3d nodes.
std::shared_ptr<EnvironmentNode> createEnvironmentNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<HrtfData> hrtf);
std::shared_ptr<Node> createSourceNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<EnvironmentNode> environment);

}