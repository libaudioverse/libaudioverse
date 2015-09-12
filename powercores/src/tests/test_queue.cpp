/**This file is part of powercores, released under the terms of the Unlicense.
See LICENSE in the root of the powercores repository for details.*/

#include <powercores/threadsafe_queue.hpp>
#include <thread>
#include <atomic>
#include <stdio.h>

bool basic_test() {
	printf("Testing queue for single-thread access...\n");
	powercores::ThreadsafeQueue<unsigned int> q;
	unsigned int length = 100;
	for(unsigned int i = 0; i < length; i++) {
		q.enqueue(i);
	}
	for(unsigned int i = 0; i < length; i++) {
		if(i != q.dequeue()) {
			printf("single-thread access failed.\n");
			return false;
		}
	}
	printf("single-thread access thest passed.\n");
	return true;
}

bool multithreaded_test() {
	printf("Testing queue for thread safety\n");
	powercores::ThreadsafeQueue<int> q1, q2;
	std::vector<std::thread> thread_array;
	unsigned int total = 50000;
	unsigned int threads = 10;
	unsigned int expectedResult = total*threads;
	std::atomic<int> accumulator;
	auto stage1 = [&]() {
		for(unsigned int i = 0; i < total; i++) q2.enqueue(q1.dequeue());
	};
	auto stage2 = [&]() {
		for(unsigned int i = 0; i < total; i++) accumulator.fetch_add(q2.dequeue());
	};
	//make pairs of stage1 and stage2 threads.
	for(unsigned int i = 0; i < threads; i++) {
		thread_array.emplace_back(std::thread(stage1));
		thread_array.emplace_back(std::thread(stage2));
	}
	//push a bunch of ints to q1.
	for(unsigned int i = 0; i < threads*total; i++) {
		q1.enqueue(1);
	}
	//wait on all of them...
	for(auto &i: thread_array) {
		i.join();
	}
	if(accumulator.load() == expectedResult) {
		printf("Thread safety test passed.\n");
		return true;
	}
	printf("Thread safety test failed.\n");
	return false;
}

#define TEST(t) if(t() == false) return;
void main() {
	TEST(basic_test);
	TEST(multithreaded_test);
}
