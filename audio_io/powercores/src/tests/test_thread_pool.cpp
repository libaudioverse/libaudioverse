/**This file is part of powercores, released under the terms of the Unlicense.
See LICENSE in the root of the powercores repository for details.*/

#include <powercores/thread_pool.hpp>
#include <thread>
#include <atomic>
#include <chrono>
#include <stdio.h>

bool basic_test() {
	printf("Performing basic thread pool test.\n");
	int threads = 10;
	int jobs = 500000;
	powercores::ThreadPool tp{threads};
	tp.start();
	std::atomic<int> accum;
	accum.store(0);
	for(int i = 0; i < jobs; i++) {
		tp.submitJob([&] () {
			accum.fetch_add(1);
		});
	}
	tp.stop();
	if(accum.load() != jobs) {
		printf("Basic test failed.  Missing jobs.\n");
		return false;
	}
	else {
		printf("Basic test passed.\n");
		return true;
	}
}

bool result_test() {
	printf("Performing test of submitJobWithResult\n");
	int threads = 10;
	int jobs = 50000;
	int accum = 0;
	std::vector<std::future<int>> futures;
	powercores::ThreadPool tp{threads};
	auto job = [] (int arg) {return arg;};
	tp.start();
	for(int i = 0; i < jobs; i++) {
		futures.emplace_back(tp.submitJobWithResult(job, 1));
	}
	for(auto &i: futures) {
		i.wait();
		accum += i.get();
	}
	tp.stop();
	if(accum == jobs) {
		printf("submitJobWithResult test passed.\n");
		return true;
	}
	else {
		printf("SubmitJobWithResult test failed.\n");
		return false;
	}
}

bool barrier_test() {
	printf("Testing barrier support...\n");
	int threads = 10;
	int iterations = 1000;
	int jobsPerIteration = 50;
	std::atomic<int> accum;
	accum.store(0);
	powercores::ThreadPool tp{threads};
	tp.start();
	for(int iteration = 0; iteration < iterations; iteration++) {
		accum.store(0);
		for(int i = 0; i < jobsPerIteration; i++) {
			tp.submitJob([&](){
				accum.fetch_add(1);
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			});
		}
		tp.submitBarrier();
		auto f = tp.submitJobWithResult([] () {}); //A dummy job that needs to run after the barrier.
		f.wait();
		if(accum.load() != jobsPerIteration) {
			printf("Barier test failed.\n");
			return false;
		}
	}
	printf("Barrier test passed.\n");
	return true;
}

#define TEST(t) if(t() == false) return;
void main() {
	TEST(basic_test);
	TEST(result_test);
	TEST(barrier_test);
}
