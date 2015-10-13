#pragma once
#include <thread>
#include <future>
#include <type_traits>
#include <atomic>
#include <functional>
#include <memory>
#include <windows.h>
#include "logging.hpp"

namespace audio_io {
namespace implementation {

/**This file contains a bunch of helper functions and macros and such for COM on windows.

We want to avoid a dependency on the ATL (introduces some dlls before vs2015) if possible, so we duplicate some functionality.*/

/**A single-threaded apartment (as pertaining to Microsoft COM) consists of a message loop and COM initialization.

This class encapsulates a single-threaded apartment and the thread to manage it.*/

class SingleThreadedApartment {
	public:
	SingleThreadedApartment();
	~SingleThreadedApartment();
	
	/*Run a call in the apartment.
	This algorithm is threadsafe, but performance may suffer under contension.*/
	template<typename FuncT, typename... ArgsT>
	typename std::result_of<FuncT(ArgsT...)>::type callInApartment(FuncT callable, ArgsT... args) {
		std::promise<typename std::result_of<FuncT(ArgsT...)>::type> promise;
		auto future = promise.get_future();
		std::function<typename std::result_of<FuncT(ArgsT...)>::type (ArgsT...)> wrappedCallable = callable;
		std::function<void(void)> task = [&] ()->void {
			promise.set_value(wrappedCallable(args...));
		};
		submitTask(task);
		future.wait();
		return future.get();
	}	
	
	private:
	//Send a task to the thread.
	//Caller needs to keep task alive until after it executes.
	void submitTask(std::function<void(void)> &task);
	//Sends a message to the apartment's thread.
	void sendMessage(UINT msg, WPARAM wparam, LPARAM lparam);
	//The thread.
	void apartmentThreadFunction();
	//Goes null when no task is being worked on.
	std::atomic<std::function<void(void)>*> current_task;
	//Goes to 1 once the apartment thread has made the message queue.
	std::atomic<int> ready;
	std::thread apartment_thread;
	DWORD apartment_thread_id;
};

//Call the specified callable with the specified args (must be at least one)in apartment ast.
//Use this by doing SingleThreadedApartment ast, and then APARTMENTCALL(func, arg1, arg2, arg3,...argn).
#define APARTMENTCALL(func, ...) (sta.callInApartment([&] () {return func(__VA_ARGS__);}))

template<typename T>
typename std::shared_ptr<typename T> wrapComPointer(T* what) {
	if(what == nullptr) {
		logDebug("audio_io", "Attempt to wrap null COM pointer.");
		return nullptr;
	}
	return std::shared_ptr<typename T>(what, [](T* p) {
		p->Release();
	});
}

}
}