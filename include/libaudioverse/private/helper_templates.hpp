/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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