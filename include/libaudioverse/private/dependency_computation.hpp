/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "node.hpp"
#include "simulation.hpp"
#include "../3d/environment.hpp"
#include "../3d/source.hpp"
#include <memory>

namespace libaudioverse_implementation {

/**Planning  used to be too slow and involve a bunch of temporaries.

Instead, everything was moved here.

To create a node that overrides its dependency management, add a template here and then insert a case in the final template in this file.
Be sure to order the final template from most to least specific.*/

template<typename CallableT, typename... ArgsT>
void nodeVisitDependencies(std::shared_ptr<Node> start, CallableT&& callable, ArgsT&&... args) {
	for(int i = 0; i < start->getInputConnectionCount(); i++) {
		auto conn = start->getInputConnection(i)->getConnectedNodes();
		for(auto &p: conn) {
			auto j = std::dynamic_pointer_cast<Job>(p->shared_from_this());
			callable(j, args...);
		}
	}
	for(auto &p: start->properties) {
		auto &prop = p.second;
		auto conn = prop.getInputConnection();
		if(conn) {
			for(auto n: conn->getConnectedNodes()) {
				auto j = std::dynamic_pointer_cast<Job>(n->shared_from_this());
				callable(j, args...);
			}
		}
	}	
}

template<typename CallableT, typename... ArgsT>
void subgraphNodeVisitDependencies(std::shared_ptr<SubgraphNode> start, CallableT&& callable, ArgsT&&... args) {
	auto j = std::static_pointer_cast<Job>(start->subgraph_output);
	if(j) callable(j, args...);
}

template<typename CallableT, typename... ArgsT>
void environmentVisitDependencies(std::shared_ptr<Node> start, CallableT&& callable, ArgsT&&... args) {
	subgraphVisitDependencies(std::static_pointer_cast<SubgraphNode>(start), callable, args...);
	//Other dependencies: all our sources.
	for(auto w: start->sources) {
		auto n = w.lock();
		if(n) {
			auto j = std::static_pointer_cast<Job>(n);
			callable(j, args...);
		}
	}
}

template<typename CallableT, typename ArgsT...>
void sourceVisitDependencies(std::shared_ptr<Node> start, CallableT&& callable, ArgsT&&... args) {
	if(start->getState() != Lav_NODESTATE_PAUSED && start->culled) visitDependencies(start->input, callable, args...);
}

#define TRY(type, name)  if(std::dynamic_pointer_cast<type>(start)) {name(std::static_pointer_cast<type>(start), callable, args...);return}

template<typename JobT, typename CallableT, typename... ArgsT>
void visitDependencies(JobT start, callableT&& callable, ArgsT&&... args) {
	TRY(Simulation, simulationVisitDependencies
	TRY(EnvironmentNode, environmentVisitDependencies)
	TRY(SoruceNode, sourceVisitDependencies)
	TRY(SubgraphNode, subgraphVisitDependencies)
	TRY(node, nodeVisitDependencies)
}

}
