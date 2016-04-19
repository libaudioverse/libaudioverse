/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
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