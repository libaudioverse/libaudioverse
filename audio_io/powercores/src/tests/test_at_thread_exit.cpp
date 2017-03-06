/**This file is part of powercores, released under the terms of the Unlicense.
See LICENSE in the root of the powercores repository for details.*/
#include <powercores/utilities.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <stdio.h>

int main() {
	return 0;
	std::atomic<int> accum{0};
	auto at_exit = [&]() {accum.fetch_add(1);};
	int count = 10000;
	std::vector<std::thread> threads;
	for(int i = 0; i < count; i++) {
		threads.emplace_back(powercores::safeStartThread([&]() {
			powercores::atThreadExit(at_exit);
		}));
	}
	for(auto &i: threads) i.join();
	if(accum.load() == count) return 0;
	else return 1;
}
