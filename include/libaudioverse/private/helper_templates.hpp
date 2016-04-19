/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once

namespace libaudioverse_implementation {

//Contains helper templates.

//Run over a container calling the specified callable on each item.
//If the callable returns false, get rid of the item.
template<typename ContainerT, typename CallableT, typename... ArgsT>
void filter(ContainerT &&container, CallableT &&callable, ArgsT&&... args) {
	auto cur = container.begin();
	while(cur != container.end()) {
		auto removing = callable(*cur, args...) == false;
		if(removing) cur = container.erase(cur);
		else cur++;
	}
}

//Like filter, but for containers of weak pointers.
//Removes all dead weak pointers, and calls callable with shared pointers for the alive ones.
//Ignores the result of callable.
template<typename ContainerT, typename CallableT, typename... ArgsT>
void filterWeakPointers(ContainerT &&container, CallableT &&callable, ArgsT&&... args) {
	auto cur = container.begin();
	while(cur != container.end()) {
		auto strong = cur->lock();
		if(strong) {
			callable(strong, args...);
			cur++;
		}
		else cur = container.erase(cur);
	}
}

//Kills the dead weak pointers in the specified container.
template<typename ContainerT>
void killDeadWeakPointers(ContainerT &&container) {
	auto cur = container.begin();
	while(cur != container.end()) {
		if(cur->lock()) cur++;
		else cur = container.erase(cur);
	}
}

}