/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/node.hpp"
#include <memory>
#include <set>


namespace libaudioverse_implementation {

class EnvironmentNode;
class EnvironmentInfo;
class Node;

class SourceNode: public SubgraphNode {
	public:
	SourceNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<EnvironmentNode> environment);
	void update(EnvironmentInfo &env);
	//involves shared pointers.
	void forwardProperties();
	void visitDependencies(std::function<void(std::shared_ptr<Job>&)> &pred) override;
	private:
	bool culled = false;
	std::shared_ptr<Node> panner_node, input;
	std::shared_ptr<EnvironmentNode> environment;
};

}