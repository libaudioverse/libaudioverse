#include <powercores/exceptions.hpp>
#include <powercores/threadsafe_queue.hpp>
#include <powercores/utilities.hpp>
#include <powercores/thread_pool.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <queue>
#include <atomic>
#include <future>
#include <type_traits>
#include <system_error>


namespace powercores {

ThreadPool::ThreadPool(int threadCount): thread_count(threadCount) {
	running.store(0);
}

ThreadPool::~ThreadPool() {
	if(running.load()) stop();
}

void ThreadPool::start() {
	running.store(1);
	job_queues.resize(thread_count);
	for(auto &i: job_queues) i = new ThreadsafeQueue<std::function<void(void)>>();
	for(int i = 0; i < thread_count; i++) {
		threads.emplace_back(safeStartThread(&ThreadPool::workerThreadFunction, this, i));
	}
}

void ThreadPool::stop() {
	for(auto &i: job_queues) i->enqueue([] () {throw ThreadPoolPoisonException();});
	running.store(0);
	for(int i = 0; i < threads.size(); i++) {
		threads[i].join();
	}
	threads.clear();
	for(auto &i: job_queues) delete i;
	job_queues.clear();
	job_queue_pointer = 0;
}

void ThreadPool::setThreadCount(int n) {
	bool wasRunning = running.load() == 1;
	if(wasRunning)  stop();
	thread_count = n;
	if(wasRunning) start();
}

void ThreadPool::submitBarrier() {
	//Promises are not copyable, so we save a pointer and delete it later, after the barrier.
	auto promise = new std::promise<void>();
	std::shared_future<void> future(promise->get_future());
	auto counter = new std::atomic<int>();
	counter->store(0);
	int goal = thread_count;
	auto barrierJob = [counter, promise, future, goal] () {
		int currentCounter = counter->fetch_add(1); //increment by one and get the current counter.
		if(currentCounter == goal-1) { //we're finished.
			promise->set_value();
			delete promise; //we're done, this isn't needed anymore.
			//We got here, so we're the last one to manipulate the counter.
			delete counter;
		}
		else {
			//Otherwise, we wait for all the other barriers.
			future.wait();
		}
	};
	for(int i = 0; i < thread_count; i++) submitJob(barrierJob);
}

void ThreadPool::workerThreadFunction(int id) {
	ThreadsafeQueue<std::function<void(void)>> &job_queue = *job_queues[id];
	int jobsSize = 5;
	std::function<void(void)> jobs[5];
	try {
		while(true) {
			int got = job_queue.dequeueRange(jobsSize, jobs);
			for(int i = 0; i < got; i++) jobs[i]();
		}
	}
	catch(ThreadPoolPoisonException) {
		//Nothing, just a way to break out.
	}
}

}