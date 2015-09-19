/**This file is part of powercores, released under the terms of the Unlicense.
See LICENSE in the root of the powercores repository for details.*/
#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <queue>
#include <atomic>
#include <future>
#include <type_traits>
#include <system_error>
#include "exceptions.hpp"
#include "threadsafe_queue.hpp"
#include "utilities.hpp"

namespace powercores {

/**Throwing this on a ThreadPool thread causes the thread to die.*/
class ThreadPoolPoisonException {
};

/**A pool of threads.  Accepts tasks in a fairly obvious manner.*/
class ThreadPool {
	public:
	ThreadPool(int count);
	~ThreadPool();
	void start();
	void stop() ;
	void setThreadCount(int n) ;
	
	/**Submit a job, which will be called in the future.
	This is a template so that we can sometimes avoid copying internally.*/
	template<typename CallableT>
	void submitJob(CallableT&& job) {
		auto &job_queue = job_queues[job_queue_pointer];
		job_queue->enqueue(job);
		job_queue_pointer = (job_queue_pointer+1)%thread_count;
	}

	/**Submit a job, possibly with arguments, to all threads.*/
	template<typename CallableT, typename... ArgsT>
	void submitJobToAllThreads(CallableT &&callable, ArgsT&&... args) {
		auto job = [callable, args...]() mutable {
			callable(args...);
		};
		for(auto &i: job_queues) i->enqueue(job);
	}
	
	/**Submit a job represented by a function with arguments and a return value, obtaining a future which will later contain the result of the job.*/
	template<class FuncT, class... ArgsT>
	std::future<typename std::result_of<FuncT(ArgsT...)>::type> submitJobWithResult(FuncT &&callable, ArgsT&&... args) {
		//The task is not copyable, so we keep a pointer and delete it after we execute it.
		auto task = new std::packaged_task<typename std::result_of<FuncT(ArgsT...)>::type(ArgsT...)>(callable);
		auto job = [task, args...] () mutable {
			(*task)(args...);
			delete task;
		};
		auto retval = task->get_future();
		submitJob(job);
		return retval;
	}
	
	/**Submit a range of jobs which will be started in order as threads become available from begin to end.*/
	template<class IterT>
	void submitJobRange(IterT begin, IterT end) {
		for(; begin != end; begin++) submitJob(*begin);
	}

	/**Submit a range of jobs.
	The jobs will run in some unspecified order.  This is faster than submitJobRange.
	
	The iterators must be random access iterators.*/
	template<class IterT>
	void submitJobRangeUnordered(IterT begin, IterT end) {
		int size = end-begin;
		if(size == 0) return;
		int perThread = size/thread_count;
		//We're giving some to all threads, we don't need to update the job_queue_pointer.
		//If we did, it'd just be what it was when we started.
		for(int i = 0; i < thread_count; i++) {
			job_queues[(job_queue_pointer+i)%thread_count]->enqueueRange(begin, begin+perThread);
			begin+=perThread;
		}
		//Submit the rest normally.
		submitJobRange(begin, end);
	}
	
	/**Map a function over a range specified by two iterators.
	The function receives the result of dereferencing the iterator and any additional arguments, and will run in some unspecified order.  The iterators must be random access.*/
	template<typename CallableT, typename IterT, typename... ArgsT>
	void map(CallableT &&callable, IterT begin, IterT end, ArgsT&&... args) {
		auto executor = [callable, args...](IterT subrangeBegin, IterT subrangeEnd) {
			for(; subrangeBegin != subrangeEnd; subrangeBegin++) callable(*subrangeBegin, args...);
		};
		int amount = end-begin;
		int amountPerThread = amount/thread_count;
		for(int i = 0; i < thread_count; i++) {
			submitJobWithResult(executor, begin, begin+amountPerThread);
			begin += amountPerThread;
		}
		if(begin != end) submitJobWithResult(executor, begin, end);
	}
	
	/**Submit a barrier.	
	A barrier ensures that all jobs enqueued before the barrier will finish execution before any job after the barrier begins execution.*/
	void submitBarrier() ;
	
	private:
	
	void workerThreadFunction(int id);
	
	//job_queue_pointer is the queue we're writing into.
	int thread_count = 0, job_queue_pointer = 0;
	std::vector<std::thread> threads;
	std::vector<ThreadsafeQueue<std::function<void(void)>>*> job_queues;
	std::atomic<int> running;
};

}