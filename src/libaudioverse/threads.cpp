#include <mutex>
#include <libaudioverse/private_all.h>
using namespace std;

extern "C" LavError createMutex(LavMutex **destination) {
	WILL_RETURN(LavError);
	recursive_mutex *retval = new recursive_mutex();
	CHECK_NOT_NULL(retval);
	*destination = (LavMutex*)retval;
	RETURN(Lav_ERROR_NONE);
	BEGIN_CLEANUP_BLOCK
	DO_ACTUAL_RETURN;
}

extern "C" LavError freeMutex(LavMutex *m) {
	recursive_mutex *mut = (recursive_mutex*)m;
	delete mut;
	return Lav_ERROR_NONE;
}

extern "C" LavError lockMutex(LavMutex *m) {
	((recursive_mutex*)m)->lock();
	return Lav_ERROR_NONE;
}

extern "C" LavError unlockMutex(LavMutex *m) {
	((recursive_mutex*)m)->unlock();
	return Lav_ERROR_NONE;
}
