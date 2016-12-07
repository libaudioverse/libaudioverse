/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
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