/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once
#include "../private/node.hpp"
#include "../private/multichannel_filter_bank.hpp"
#include "../implementations/biquad.hpp"
#include <memory>

namespace libaudioverse_implementation {

/**Note.  We can't use floats. There's some instability with the accumulator model that was here before that shows up as audible artifacts.*/
class ThreeBandEqNode: public Node {
	public:
	ThreeBandEqNode(std::shared_ptr<Server> server, int channels);
	void recompute();
	virtual void process() override;
	virtual void reset() override;
	float lowband_gain;
	MultichannelFilterBank<BiquadFilter> midband_peaks, highband_shelves;
};

std::shared_ptr<Node> createThreeBandEqNode(std::shared_ptr<Server> server, int channels);
}