/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once
#include "../private/node.hpp"
#include "../private/callback.hpp"
#include <memory>
namespace libaudioverse_implementation {

class Server;
class Node;

class CrossfaderNode: public Node {
	public:
	CrossfaderNode(std::shared_ptr<Server> sim, int channels, int inputs);
	void crossfade(float duration, int input);
	//Immediately finish the current crossfade.
	void finishCrossfade();
	void process();
	std::shared_ptr<Callback<void()>> finished_callback;
	private:
	int channels = 0;
	int current = 0, target = 0;
	float current_weight = 1.0, target_weight = 0.0, delta = 0.0;
	bool crossfading = false;
};

std::shared_ptr<Node> createCrossfaderNode(std::shared_ptr<Server> server, int channels, int inputs);

}