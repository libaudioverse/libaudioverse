/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once
#include "../private/node.hpp"
#include "../implementations/iir.hpp"
#include <random>
#include <memory>

namespace libaudioverse_implementation {

class NoiseNode: public Node {
	public:
	NoiseNode(std::shared_ptr<Simulation> simulation);
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

std::shared_ptr<Node> createNoiseNode(std::shared_ptr<Simulation> simulation);
}