/**This file is part of powercores, released under the terms of the Unlicense.
See LICENSE in the root of the powercores repository for details.*/

#include <powercores/thread_pool.hpp>
#include <thread>
#include <atomic>
#include <chrono>
#include <stdio.h>

int main() {
	printf("Testing barrier support...\n");
	int threads = 10;
	int iterations = 1000;
	int jobsPerIteration = 50;
	std::atomic<int> accum{0};
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
			return 1;
		}
	}
	printf("Barrier test passed.\n");
	return 0;
}
