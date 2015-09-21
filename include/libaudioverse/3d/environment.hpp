/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../libaudioverse.h"
#include <vector>
#include <set>
#include <memory>
#include <tuple>
#include <glm/glm.hpp>

namespace libaudioverse_implementation {

class SourceNode;
class HrtfData;
class Simulation;
class Buffer;

/**Configuration of an effect send.*/
class EffectSendConfiguration {
	public:
	int channels = 0; //1, 2, 4, 6, or 8.
	bool is_reverb = false; //For 4-channel effect sends, if we should use reverb-type panning.
	bool connect_by_default = false; //If sources should be connected to this send by default.
};

/**This holds info on listener positions, defaults, etc.
Anything a source needs for updating, basically.*/
class EnvironmentInfo {
	public:
	glm::mat4 world_to_listener_transform;
};

class EnvironmentNode: public SubgraphNode {
	public:
	EnvironmentNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<HrtfData> hrtf);
	void registerSourceForUpdates(std::shared_ptr<SourceNode> source, bool useEffectSends = true);
	//Maybe change our output channels.
	//Also update sources, which might reconfigure themselves.
	virtual void willTick() override;
	std::shared_ptr<HrtfData> getHrtf();
	//Play buffer asynchronously at specified position, destroying the source when done.
	void playAsync(std::shared_ptr<Buffer> buffer, float x, float y, float z, bool isDry = false);
	//Get the output.
	//This is needed for effect sends, which must jump directly to it.
	std::shared_ptr<Node> getOutputNode();
	//Manage effect sends.
	//Returns the integer identifier of the send.
	int addEffectSend(int channels, bool isReverb, bool connecctByDefault);
	EffectSendConfiguration& getEffectSend(int which);
	private:
	//while these may be parents (through virtue of the panners we give out), they also have to hold a reference to us-and that reference must be strong.
	//the world is more capable of handling a source that dies than a source a world that dies.
	std::set<std::weak_ptr<SourceNode>, std::owner_less<std::weak_ptr<SourceNode>>> sources;
	std::shared_ptr<HrtfData > hrtf;
	std::shared_ptr<Node> output=nullptr;
	EnvironmentInfo environment_info;
	std::vector<EffectSendConfiguration> effect_sends;
	
	template<typename JobT, typename CallableT, typename... ArgsT>
	friend void environmentVisitDependencies(JobT&& start, CallableT &&callable, ArgsT&&... args);
	//This is used to make play_async not invalidate the plan.
	std::vector<std::tuple<std::shared_ptr<Node>, std::shared_ptr<SourceNode>>> play_async_source_cache;
	int play_async_source_cache_limit = 30; //How many we're willing to cache.
};

std::shared_ptr<EnvironmentNode> createEnvironmentNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<HrtfData> hrtf);
}