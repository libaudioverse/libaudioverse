/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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