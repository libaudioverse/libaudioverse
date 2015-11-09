/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/node.hpp"
#include "../implementations/amplitude_panner.hpp"
#include <memory>

namespace libaudioverse_implementation {

class AmplitudePannerNode: public Node {
	public:
	AmplitudePannerNode(std::shared_ptr<Simulation> simulation);
	virtual void process() override;
	void recomputeChannelMap();
	void configureStandardChannelMap(unsigned int channels);
	bool map_changed = true, has_center = false, has_lfe = false, skip_center = false, skip_lfe = false;
	AmplitudePanner panner;
};

std::shared_ptr<Node>createAmplitudePannerNode(std::shared_ptr<Simulation> simulation);
}