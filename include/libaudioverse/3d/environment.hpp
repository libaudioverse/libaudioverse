/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
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
	int start = 0; //The output we start at.
	bool is_reverb = false; //For 4-channel effect sends, if we should use reverb-type panning.
	bool connect_by_default = false; //If sources should be connected to this send by default.
};

/**This holds info on listener positions, defaults, etc.
Anything a source needs for updating, basically.*/
class EnvironmentInfo {
	public:
	glm::mat4 world_to_listener_transform;
	//These avoid tons of property lookups.
	//Each lookup on the environment is a shared_ptr indirection and a dictionary lookup.
	int distance_model, panning_strategy;
};

/**The sorce and environment model does not use the standard node and implementation separation.

Sources write directly to special buffers in the environment, which are then copied to the environment's output in the process method.
*/

class EnvironmentNode: public Node {
	public:
	EnvironmentNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<HrtfData> hrtf);
	~EnvironmentNode();
	void registerSourceForUpdates(std::shared_ptr<SourceNode> source, bool useEffectSends = true);
	//Maybe change our output channels.
	//Also update sources, which might reconfigure themselves.
	virtual void willTick() override;
	virtual void process() override;
	std::shared_ptr<HrtfData> getHrtf();
	//Play buffer asynchronously at specified position, destroying the source when done.
	void playAsync(std::shared_ptr<Buffer> buffer, float x, float y, float z, bool isDry = false);
	//Manage effect sends.
	//Returns the integer identifier of the send.
	int addEffectSend(int channels, bool isReverb, bool connecctByDefault);
	EffectSendConfiguration& getEffectSend(int which);
	int getEffectSendCount();
	//This is a public variable; sources write directly to these buffers.
	//There are always at least 8 buffers, with additional buffers appended for effect sends.
	std::vector<float*> source_buffers;
	private:
	//while these may be parents (through virtue of the panners we give out), they also have to hold a reference to us-and that reference must be strong.
	//the world is more capable of handling a source that dies than a source a world that dies.
	std::set<std::weak_ptr<SourceNode>, std::owner_less<std::weak_ptr<SourceNode>>> sources;
	std::shared_ptr<HrtfData > hrtf;
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