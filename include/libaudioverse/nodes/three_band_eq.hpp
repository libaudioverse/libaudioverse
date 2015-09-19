/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/node.hpp"
#include "../private/multichannel_filter_bank.hpp"
#include "../implementations/biquad.hpp"
#include <memory>

namespace libaudioverse_implementation {

/**Note.  We can't use floats. There's some instability with the accumulator model that was here before that shows up as audible artifacts.*/
class ThreeBandEqNode: public Node {
	public:
	ThreeBandEqNode(std::shared_ptr<Simulation> simulation, int channels);
	void recompute();
	virtual void process() override;
	virtual void reset() override;
	float lowband_gain;
	MultichannelFilterBank<BiquadFilter> midband_peaks, highband_shelves;
};

std::shared_ptr<Node> createThreeBandEqNode(std::shared_ptr<Simulation> simulation, int channels);
}