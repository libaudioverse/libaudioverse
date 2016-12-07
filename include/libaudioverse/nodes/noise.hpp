/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once
#include "../private/node.hpp"
#include "../implementations/iir.hpp"
#include <random>
#include <memory>

namespace libaudioverse_implementation {

class NoiseNode: public Node {
	public:
	NoiseNode(std::shared_ptr<Server> server);
	virtual void process();
	void white();
	void pink();
	void brown();
	std::minstd_rand random_number_generator;
	std::normal_distribution<float> normal_distribution;
	IIRFilter pinkifier; //filter to turn white noise into pink noise.
	IIRFilter brownifier; //and likewise for brown.
	float pink_max = 0.0f, brown_max = 0.0f; //used for normalizing noise.
};

std::shared_ptr<Node> createNoiseNode(std::shared_ptr<Server> server);
}