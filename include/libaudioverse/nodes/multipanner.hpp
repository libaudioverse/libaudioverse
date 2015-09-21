/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/node.hpp"
#include <memory>

namespace libaudioverse_implementation {

class Simulation;
class HrtfData;

class MultipannerNode: public SubgraphNode {
	public:
	MultipannerNode(std::shared_ptr<Simulation> sim, std::shared_ptr<HrtfData> hrtf);
	std::shared_ptr<Node> hrtf_panner = nullptr, amplitude_panner = nullptr, input= nullptr, current_panner = nullptr;
	void configureForwardedProperties();
	void strategyChanged();
	void willTick() override;
	void reset() override;
};

std::shared_ptr<Node> createMultipannerNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<HrtfData> hrtf);
}