/**This file is part of powercores, released under the terms of the Unlicense.
See LICENSE in the root of the powercores repository for details.*/

#include <powercores/thread_pool.hpp>
#include <thread>
#include <atomic>
#include <chrono>
#include <stdio.h>

int main() {
	printf("Performing basic thread pool test.\n");
	int threads = 10;
	int jobs = 500000;
	powercores::ThreadPool tp{threads};
	tp.start();
	std::atomic<int> accum{0};
	for(int i = 0; i < jobs; i++) {
		tp.submitJob([&] () {
			accum.fetch_add(1);
		});
	}
	tp.stop();
	if(accum.load() != jobs) {
		printf("Basic test failed.  Missing jobs.\n");
		return 1;
	}
	else {
		printf("Basic test passed.\n");
		return 0;
	}
}
