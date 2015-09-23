/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/node.hpp"
#include <memory>
#include <set>
#include <vector>
#include <map>

namespace libaudioverse_implementation {

class EnvironmentNode;
class EnvironmentInfo;
class Node;

class SourceNode: public SubgraphNode {
	public:
	SourceNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<EnvironmentNode> environment);
	~SourceNode();
	void forwardProperties(); //involves shared_from_this.
	void reset() override;
	//For the case of 1 channels, returns the input gain node as-is.
	std::shared_ptr<Node> getPannerForEffectChannels(int channels);
	void feedEffect(int which);
	void stopFeedingEffect(int which);
	void update(EnvironmentInfo &env);
	void handleStateUpdates(bool shouldCull);
	void handleOcclusion();
	private:
	bool culled = false;
	std::shared_ptr<Node> panner_node, input, occluder;
	std::shared_ptr<EnvironmentNode> environment;
	std::vector<std::shared_ptr<Node>> effect_panners;
	//It is unlikely that we are going to have more effect sends than possible gain nodes.
	//We are either sending to a regular or a reverb effect.
	std::map<int, std::shared_ptr<Node>> outgoing_effects, outgoing_effects_reverb;
	
	template<typename JobT, typename CallableT, typename... ArgsT>
	friend void sourceVisitDependencies(JobT&& start, CallableT &&callable, ArgsT&&... args);
};

std::shared_ptr<Node> createSourceNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<EnvironmentNode> environment);
}