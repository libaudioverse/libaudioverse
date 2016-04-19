/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
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

template<typename  JobT, typename CallableT, typename... ArgsT>
inline void simulationVisitDependencies(JobT&& start, CallableT&& callable, ArgsT&&... args) {
	start->final_output_connection->visitInputs(callable, args...);
	filterWeakPointers(start->always_playing_nodes, [](std::shared_ptr<Node> &n, CallableT &callable, ArgsT&&... args2) {
		auto j = std::static_pointer_cast<Job>(n);
		callable(j, args2...);
	}, callable, args...);
}

template<typename JobT, typename CallableT, typename... ArgsT>
inline void nodeVisitDependencies(JobT&& start, CallableT&& callable, ArgsT&&... args) {
	for(int i = 0; i < start->getInputConnectionCount(); i++) {
		start->getInputConnection(i)->visitInputs(callable, args...);
	}
	for(auto &p: start->properties) {
		auto &prop = p.second;
		auto conn = prop.getInputConnection();
		if(conn) conn->visitInputs(callable, args...);
	}	
}

template<typename JobT, typename CallableT, typename... ArgsT>
inline void environmentVisitDependencies(JobT&& start, CallableT&& callable, ArgsT&&... args) {
	//dependencies: all our sources.
	for(auto w: start->sources) {
		auto n = w.lock();
		if(n) {
			auto j = std::static_pointer_cast<Job>(n);
			callable(j, args...);
		}
	}
}


#define TRY(type, name)  auto casted##type = std::dynamic_pointer_cast<type>(start); if(casted##type) {name(casted##type, callable, args...);return;}

template<typename JobT, typename CallableT, typename... ArgsT>
inline void visitDependencies(JobT &&start, CallableT&& callable, ArgsT&&... args) {
	TRY(Simulation, simulationVisitDependencies)
	TRY(EnvironmentNode, environmentVisitDependencies)
	TRY(Node, nodeVisitDependencies)
}

}
