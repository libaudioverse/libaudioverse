#include <mutex>
#include <thread>
#include <chrono>
#include <libaudioverse/private_all.h>
using namespace std;

Lav_PUBLIC_FUNCTION LavError createMutex(void **destination) {
	WILL_RETURN(LavError);
	recursive_mutex *retval = new recursive_mutex();
	CHECK_NOT_NULL(retval);
	*destination = (void*)retval;
	RETURN(Lav_ERROR_NONE);
	BEGIN_CLEANUP_BLOCK
	DO_ACTUAL_RETURN;
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
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(destination);
	CHECK_NOT_NULL(func);
	thread *t = new thread(func, param);
	CHECK_NOT_NULL(t);
	BEGIN_CLEANUP_BLOCK
	DO_ACTUAL_RETURN;
}

Lav_PUBLIC_FUNCTION LavError threadJoinAndFree(void* t) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(t);
	((thread*)t)->join();
	delete (thread*)t;
	RETURN(Lav_ERROR_NONE);
	BEGIN_CLEANUP_BLOCK
	DO_ACTUAL_RETURN;
}

Lav_PUBLIC_FUNCTION void sleepFor(unsigned int milliseconds) {
	this_thread::sleep_for(chrono::milliseconds(milliseconds));
}

Lav_PUBLIC_FUNCTION void yield() {
	this_thread::yield();
}
