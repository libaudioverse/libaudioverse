/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../libaudioverse.h"
#include "memory.hpp"
#include <functional>
#include <memory>

namespace libaudioverse_implementation {

/**Helper template encapsulating a callback.*/

template<typename IgnoredT>
class Callback;

template<typename ResultT, typename... ArgsT>
class Callback<ResultT (ArgsT...)> {
	public:
	//use the default construction of the type, if possible.
	Callback(): Callback([] () {}) {}
	//We specify the callable. This is an exact match.
	Callback(std::function<ResultT()> _default): default(_default) {}
	
	ResultT operator()(ArgsT... args) {
	if(callback) return callback(args...);
	else return default();
	}
	
	void setDefault(std::function<ResultT(ArgsT...)>f) {default= f;}
	void setCallback(std::function<ResultT(ArgsT...)> f) {callback = f;}
	void clear() {callback = nullptr;}
	private:
	std::function<ResultT(ArgsT...)> callback, default;
};

inline auto wrapParameterlessCallback(std::shared_ptr<ExternalObject> obj, LavParameterlessCallback cb, void* userdata) {
	auto wobj = std::weak_ptr<ExternalObject>(obj);
	return [wobj, cb, userdata]() {
		if(cb == nullptr) return;
		auto s = wobj.lock();
		if(s) cb(outgoingObject(s), userdata);
	};
}

}