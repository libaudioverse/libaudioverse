/**This file is part of powercores, released under the terms of the Unlicense.
See LICENSE in the root of the powercores repository for details.*/

#include <powercores/threadsafe_queue.hpp>
#include <thread>
#include <atomic>
#include <stdio.h>

int main() {
	printf("Testing queue for single-thread access...\n");
	powercores::ThreadsafeQueue<unsigned int> q;
	unsigned int length = 100;
	for(unsigned int i = 0; i < length; i++) {
		q.enqueue(i);
	}
	for(unsigned int i = 0; i < length; i++) {
		if(i != q.dequeue()) {
			printf("single-thread access failed.\n");
			return 1;
		}
	}
	printf("single-thread access thest passed.\n");
	return 0;
}
