/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "node.hpp"
#include "simulation.hpp"
#include "helper_templates.hpp"
#include "../3d/environment.hpp"
#include "../3d/source.hpp"
#include <memory>

namespace libaudioverse_implementation {

/**Planning  used to be too slow and involve a bunch of temporaries.

Instead, everything was moved here.

To create a node that overrides its dependency management, add a template here and then insert a case in the final template in this file.
Be sure to order the final template from most to least specific.*/

template<typename CallableT, typename... ArgsT>
void simulationVisitDependencies(std::shared_ptr<Simulation> start, CallableT&& callable, ArgsT&&... args) {
	for(auto &i: start->final_output_connection->getConnectedNodes()) {
		auto n = std::dynamic_pointer_cast<Job>(i->shared_from_this());
		if(n) callable(n, args...);
	}
	filterWeakPointers(start->always_playing_nodes, [](std::shared_ptr<Node> &n, CallableT &callable, ArgsT&&... args2) {
		auto j = std::static_pointer_cast<Job>(n);
		callable(j, args2...);
	}, callable, args...);
}

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
void environmentVisitDependencies(std::shared_ptr<EnvironmentNode> start, CallableT&& callable, ArgsT&&... args) {
	subgraphNodeVisitDependencies(std::static_pointer_cast<SubgraphNode>(start), callable, args...);
	//Other dependencies: all our sources.
	for(auto w: start->sources) {
		auto n = w.lock();
		if(n) {
			auto j = std::static_pointer_cast<Job>(n);
			callable(j, args...);
		}
	}
}

template<typename CallableT, typename... ArgsT>
void sourceVisitDependencies(std::shared_ptr<SourceNode> start, CallableT&& callable, ArgsT&&... args) {
	if(start->getState() != Lav_NODESTATE_PAUSED && start->culled) visitDependencies(start->input, callable, args...);
}

#define TRY(type, name)  auto casted##type = std::dynamic_pointer_cast<type>(start); if(casted##type) {name(casted##type, callable, args...);return;}

template<typename CallableT, typename... ArgsT>
void visitDependencies(std::shared_ptr<Job> start, CallableT&& callable, ArgsT&&... args) {
	TRY(Simulation, simulationVisitDependencies)
	TRY(EnvironmentNode, environmentVisitDependencies)
	TRY(SourceNode, sourceVisitDependencies)
	TRY(SubgraphNode, subgraphNodeVisitDependencies)
	TRY(Node, nodeVisitDependencies)
}

}
