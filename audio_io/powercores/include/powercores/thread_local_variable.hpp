/**This file is part of powercores, released under the terms of the Unlicense.
See LICENSE in the root of the powercores repository for details.*/
#pragma once
#include "utilities.hpp"
#include <atomic>
#include <thread>
#include <system_error>
#include <utility>
#include <functional>
#include <unordered_map>
#include <vector>

namespace powercores {

/**A thread-local variable. Get the value by dereferencing with *.
-> also works, if and only if the contained type implements ->.*/
template<typename T>
class ThreadLocalVariable {
	public:
	//Create a ThreadLocalVariable, and default construct the contents.
	ThreadLocalVariable(): ThreadLocalVariable([] () {return new T();}) {}
	/*Create a thread-local variable with a  custom construction function.
	This function will be called on the first access from any thread that does not yet have a value for the vaeriable.*/
	ThreadLocalVariable(std::function<T*(void)> creator): ThreadLocalVariable(creator, [](T* i) {delete i;}) {}
	
	/**Create  a thread-local variable with custom creator and deleter.
	The creator is called on the first access from any thread which has not used the variable before.
	The deleter is called from threads which have used the variable, and must obey the same limitations as a destructor (i.e. don't throw exceptions).*/
	ThreadLocalVariable(std::function<T*(void)> creator, std::function<void(T*)> deleter) {
		this->creator = creator;
		this->deleter = deleter;
		map_atomic.store(&map);
	}
	
	T& operator*() {
		auto id = getThreadId();
		acquireMap();
		int had = map.count(id);
		auto weak = map[id];
		releaseMap();
		auto strong = weak.lock();
		//Do we need to create?
		if(had == 0 || strong == nullptr) {
			auto ptr = creator();
			auto deleter = this->deleter; //So we can capture a copy.
			strong = std::shared_ptr<T>(ptr, [deleter](void* p){deleter(static_cast<T*>(p));});
			//The weak pointer will die when the thread does.
			local_strong_references.push_back(strong);
			acquireMap();
			map[id] = strong;
			releaseMap();
		}
		return *strong;
	}
	
	T& operator->() {
		//Return the result of dereferencing ourself. This works because we're a parameterless template.
		return **this;
	}
	
	private:
	void acquireMap() {
		//If we read not-nullptr then it's ours, otherwise we're spin waiting for another thread to give us the map.
		while(map_atomic.exchange(nullptr, std::memory_order_acquire) == nullptr) {
			std::this_thread::yield();
		}
	}
	
	void releaseMap() {
		//We assume we acquired first. So just:
		map_atomic.store(&map, std::memory_order_release);
	}
	
	std::atomic<std::unordered_map<long long, std::weak_ptr<T>>*> map_atomic{nullptr};
	std::unordered_map<long long, std::weak_ptr<T>> map;
	std::function<T*(void)> creator;
	std::function<void(T*)> deleter;
	//When this goes out of scope, the references break.
	thread_local static std::vector<std::shared_ptr<T>> local_strong_references;
};

template<typename T>
thread_local std::vector<std::shared_ptr<T>> ThreadLocalVariable<T>::local_strong_references;

}