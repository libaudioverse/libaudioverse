/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
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
class HrtfData;

class SourceNode: public Node {
	public:
	SourceNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<EnvironmentNode> environment);
	~SourceNode();
	void reset() override;
	void feedEffect(int which);
	void stopFeedingEffect(int which);
	int computeDistanceModel(EnvironmentInfo& env);
	void update(EnvironmentInfo &env);
	virtual void process() override;
	void handleStateUpdates(bool shouldCull);
	void handleOcclusion();
	private:
	bool culled = false;
	float dry_gain, reverb_gain;
	int panning_strategy;
	HrtfPanner hrtf_panner;
	AmplitudePanner stereo_panner, surround40_panner, surround51_panner, surround71_panner;
	BiquadFilter occlusion_filter;
	std::shared_ptr<EnvironmentNode> environment;
	std::shared_ptr<HrtfData> hrtf_data;
	std::map<int, AmplitudePanner*> fed_effects;
};

std::shared_ptr<SourceNode> createSourceNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<EnvironmentNode> environment);
}