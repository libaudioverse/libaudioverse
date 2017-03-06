/**This file is part of powercores, released under the terms of the Unlicense.
See LICENSE in the root of the powercores repository for details.*/
#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <queue>
#include <atomic>
#include "exceptions.hpp"

namespace powercores {
/**A threadsafe queue supporting any number of readers and writers.

Note: T must be default constructible, copy assignable and copy constructible.*/
template <typename T>
class ThreadsafeQueue {
	public:
	/**Enqueue an item.*/
	void enqueue(T item) {
		std::unique_lock<std::mutex> l(lock);
		internal_queue.push_front(item);
		_size++;
		enqueued_notify.notify_one();
	}

	/**Dequeue an item.
If there is no item in the queue, this function sleeps forever.*/
	T dequeue() {
		std::unique_lock<std::mutex> l(lock);
		if(internal_queue.empty() == false) {
			return actualDequeue();
		}

		enqueued_notify.wait(l, [this]() {return internal_queue.empty() == false;});
		return actualDequeue();
	}

	/**Like dequeue, but will throw TimeoutException if there is nothing to dequeue before the timeout.*/
	T dequeueWithTimeout(int timeoutInMS) {
		std::unique_lock<std::mutex> l(lock);
		if(internal_queue.empty() == false) {
			return actualDequeue();
		}
		bool res = enqueued_notify.wait_for(l, std::chrono::milliseconds(timeoutInMS), [this]() {return internal_queue.empty() == false;});
		if(res) return actualDequeue();
		else throw TimeoutException();
	}

	/**Enqueue a range represented by the iterator begin and end.*/
	template<class IterT>
	void enqueueRange(IterT begin, IterT end) {
		std::unique_lock<std::mutex> l(lock);
		for(; begin != end; begin++) internal_queue.push_front(*begin);
	}
	
	/**DequeueRange dequeues at least one item and at most the specified count, storing them in the iterator.
	The number of items dequeued is returned.*/
	template<class IterT>
	int dequeueRange(int count, IterT output) {
		std::unique_lock<std::mutex> l(lock);
		int ret = 0;
		if(internal_queue.empty()) enqueued_notify.wait(l, [this] () {return internal_queue.empty() == false;});
		while(ret < count && internal_queue.empty() == false) {
			*output = actualDequeue();
			ret++;
			output++;
		}
		return ret;
	}
	
	bool empty() {
		std::lock_guard<std::mutex> l(lock);
		return internal_queue.empty();
	}

/**Get the current number of items in the queue.*/
	unsigned int size() {
		std::lock_guard<std::mutex> l(lock);
		return _size;
	}
	
	private:
	T actualDequeue() {
		auto res = internal_queue.back();
		internal_queue.pop_back();
		_size--;
		return res;
	}
	
	std::mutex lock;
	std::deque<T> internal_queue;
	std::condition_variable enqueued_notify;
	unsigned int _size = 0;
};

}
