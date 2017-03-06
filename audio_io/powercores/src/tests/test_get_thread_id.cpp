/**This file is part of powercores, released under the terms of the Unlicense.
See LICENSE in the root of the powercores repository for details.*/
#include <powercores/utilities.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <set>
#include <vector>
#include <stdio.h>

int main() {
	std::atomic<int> failed_persistency{0};
	std::set<long long> ids;
	std::mutex ids_mutex;
	int count = 100;
	std::vector<std::thread> threads;
	for(int i = 0; i < count; i++) {
		threads.push_back(powercores::safeStartThread([&] () {
			auto id = powercores::getThreadId();
			//persistency:
			failed_persistency.fetch_add(id != powercores::getThreadId());
			std::lock_guard<std::mutex> g(ids_mutex);
			ids.insert(id);
		}));
	}
	for(auto &i: threads) i.join();
	if(failed_persistency.load()) {
		printf("Failed persistency.\n");
		return 1;
	}
	if(ids.size() != count) {
		printf("Failed to get a unique id for every thread.\n");
		return 1;
	}
	return 0;
}