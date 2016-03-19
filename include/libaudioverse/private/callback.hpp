/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../libaudioverse.h"
#include "memory.hpp"
#include <functional>
#include <memory>
#include <mutex>
#include <type_traits>

namespace libaudioverse_implementation {

/**Helper template encapsulating a callback.*/

template<typename ResultT, typename... ArgsT>
class CallbackWithoutResult;

template<typename ResultT, typename... ArgsT>
class CallbackWithoutResult<ResultT(ArgsT...)>  {
	public:
	CallbackWithoutResult() = default;
	ResultT operator()(ArgsT... args) {
		std::lock_guard<std::recursive_mutex> g(mutex);
		if(callback) callback(args...);
	}
	
	void setCallback(std::function<ResultT(ArgsT...)> f) {
		std::lock_guard<std::recursive_mutex> g(mutex);
		callback = f;
	}
	
	void clear() {
		std::lock_guard<std::recursive_mutex> g(mutex);
		callback = nullptr;
	}
	
	protected:
	std::function<ResultT(ArgsT...)> callback;
	std::recursive_mutex mutex;
};

template<typename ResultT, typename... ArgsT>
class CallbackWithResult;

template<typename ResultT, typename... ArgsT>
class CallbackWithResult<ResultT(ArgsT...)>: protected CallbackWithoutResult<ResultT(ArgsT...)> {
	public:
	ResultT operator()(ArgsT... args) override {
		std::lock_guard<std::recursive_mutex> g(this->mutex);
		if(this->callback) return this->callback(args...);
		else return default_result;
	}
	
	void setDefault(ResultT d) {
		std::lock_guard<std::recursive_mutex> g(this->mutex);
		default_result = d;
	}
	
	protected:
	ResultT default_result = {};
};

//Avoid bringing in boost. This is stupid easy, so.
template<typename ResultT, typename... ArgsT>
class ResultType;

template<typename ResultT, typename... ArgsT>
class ResultType<ResultT(ArgsT...)> {
	public:
	typedef ResultT type;
};

template<typename FuncT>
using Callback = typename std::conditional<std::is_same<typename ResultType<FuncT>::type, void>::value, CallbackWithoutResult<FuncT>, CallbackWithResult<FuncT>>::type;

inline auto wrapParameterlessCallback(std::shared_ptr<ExternalObject> obj, LavParameterlessCallback cb, void* userdata) {
	auto wobj = std::weak_ptr<ExternalObject>(obj);
	return [wobj, cb, userdata]() {
		if(cb == nullptr) return;
		auto s = wobj.lock();
		if(s) cb(outgoingObject(s), userdata);
	};
}

}