/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
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
	SourceNode(std::shared_ptr<Server> server, std::shared_ptr<EnvironmentNode> environment);
	~SourceNode();
	void reset() override;
	void feedEffect(int which);
	void stopFeedingEffect(int which);
	void update(EnvironmentInfo env);
	void updateEnvironmentInfoFromProperties(EnvironmentInfo& env);
	void updatePropertiesFromEnvironmentInfo(const EnvironmentInfo& env);
	void setPropertiesFromEnvironment();
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

std::shared_ptr<SourceNode> createSourceNode(std::shared_ptr<Server> server, std::shared_ptr<EnvironmentNode> environment);
}