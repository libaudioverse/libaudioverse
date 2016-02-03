/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/node.hpp"
#include "../implementations/amplitude_panner.hpp"
#include "../implementations/hrtf_panner.hpp"
#include "../implementations/biquad.hpp"
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
	void reset() override;
	void process() override;
	void feedEffect(int which);
	void stopFeedingEffect(int which);
	void update(EnvironmentInfo &env);
	void handleStateUpdates(bool shouldCull);
	void handleOcclusion();
	private:
	bool culled = false;
	HrtfPanner hrtf_panner;
	AmplitudePanner amplitude_panner;
	BiquadFilter occlusion_filter;
	std::shared_ptr<EnvironmentNode> environment;

	template<typename JobT, typename CallableT, typename... ArgsT>
	friend void sourceVisitDependencies(JobT&& start, CallableT &&callable, ArgsT&&... args);
};

std::shared_ptr<Node> createSourceNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<EnvironmentNode> environment);
}