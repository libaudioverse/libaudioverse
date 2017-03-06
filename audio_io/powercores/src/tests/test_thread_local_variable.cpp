/**This file is part of powercores, released under the terms of the Unlicense.
See LICENSE in the root of the powercores repository for details.*/
#include <powercores/utilities.hpp>
#include <powercores/thread_local_variable.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <stdio.h>

int main() {
	powercores::ThreadLocalVariable<int> v;
	std::atomic<int> accum{0};
	std::atomic<int> failed_persistent{0}; //Goes to one or more if a ThreadLocalVariable fails to hold contents.
	std::vector<std::thread> threads;
	int count = 100;
	int multiplier = 100;
	for(int i = 0; i < count; i++) {
		threads.push_back(powercores::safeStartThread([&] () {
			*v = 0;
			for(int i = 0; i < multiplier; i++) *v += 1;
			if(*v != 100) failed_persistent.fetch_add(1);
			accum.fetch_add(*v);
		}));
	}
	for(auto &i: threads) i.join();
	if(failed_persistent.load()) {
		printf("Failed to be persistent.\n");
		return 1;
	}
	if(accum.load() != count*multiplier) {
		printf("Failed to get all results.\n");
		return 1;
	}
	return 0;
}

