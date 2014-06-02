/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <libaudioverse/private_all.h>
using namespace std;

Lav_PUBLIC_FUNCTION LavError createMutex(void **destination) {
	recursive_mutex *retval = new recursive_mutex();
	if(destination == NULL) {
		return Lav_ERROR_NULL_POINTER;
	}
	if(retval == NULL) {
		return Lav_ERROR_MEMORY;
	}
	*destination = (void*)retval;
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError freeMutex(void *m) {
	mutex *mut = (mutex*)m;
	delete mut;
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError mutexLock(void *m) {
	((mutex*)m)->lock();
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError mutexUnlock(void *m) {
	((mutex*)m)->unlock();
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError threadRun(LavThreadCapableFunction func, void* param, void** destination) {
	if(func == NULL || destination == NULL || param == NULL) {
		return Lav_ERROR_NULL_POINTER;
	}
	thread *t = new thread(func, param);
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError threadJoinAndFree(void* t) {
	if(t == NULL) {
		return Lav_ERROR_NULL_POINTER;
	}
	((thread*)t)->join();
	delete (thread*)t;
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION void sleepFor(unsigned int milliseconds) {
	this_thread::sleep_for(chrono::milliseconds(milliseconds));
}

Lav_PUBLIC_FUNCTION void yield() {
	this_thread::yield();
}

Lav_PUBLIC_FUNCTION LavError createAFlag(void** destination) {
	void* retval = (void*)new atomic_flag();
	if(retval == NULL) {
		return Lav_ERROR_MEMORY;
	}
	*destination = retval;
	return Lav_ERROR_NONE;
}

	int aFlagTestAndSet(void* flag) {
	return ((atomic_flag*)flag)->test_and_set();
}

void aFlagClear(void* flag) {
	((atomic_flag*)flag)->clear();
}

void aFlagFree(void* flag) {
	delete (atomic_flag*)flag;
}
