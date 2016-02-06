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