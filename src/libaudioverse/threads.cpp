#include <mutex>
#include <libaudioverse/private_all.h>
using namespace std;

extern "C" LavError createMutex(void **destination) {
	WILL_RETURN(LavError);
	recursive_mutex *retval = new recursive_mutex();
	CHECK_NOT_NULL(retval);
	*destination = (void*)retval;
	RETURN(Lav_ERROR_NONE);
	BEGIN_CLEANUP_BLOCK
	DO_ACTUAL_RETURN;
}


extern "C" LavError freeMutex(void *m) {
	mutex *mut = (mutex*)m;
	delete mut;
	return Lav_ERROR_NONE;
}

extern "C" LavError lockMutex(void *m) {
	((mutex*)m)->lock();
	return Lav_ERROR_NONE;
}

extern "C" LavError unlockMutex(void *m) {
	((mutex*)m)->unlock();
	return Lav_ERROR_NONE;
}
