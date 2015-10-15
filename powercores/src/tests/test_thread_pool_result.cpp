/**This file is part of powercores, released under the terms of the Unlicense.
See LICENSE in the root of the powercores repository for details.*/

#include <powercores/thread_pool.hpp>
#include <thread>
#include <atomic>
#include <chrono>
#include <stdio.h>


int main() {
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
		return 0;
	}
	else {
		printf("SubmitJobWithResult test failed.\n");
		return 1;
	}
}
