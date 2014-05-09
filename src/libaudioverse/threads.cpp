#include <mutex>
#include <libaudioverse/private_all.h>
using namespace std;

extern "C" LavError createMutex(LavMutex **destination) {
	WILL_RETURN(LavError);
	mutex *retval = new mutex();
	CHECK_NOT_NULL(retval);
	*destination = (LavMutex*)retval;
	BEGIN_CLEANUP_BLOCK
	DO_ACTUAL_RETURN;
}

extern "C" LavError freeMutex(LavMutex *m) {
	mutex *mut = (mutex*)m;
	delete mut;
	return Lav_ERROR_NONE;
}

extern "C" LavError lockMutex(LavMutex *m) {
	((mutex*)m)->lock();
	return Lav_ERROR_NONE;
}

extern "C" LavError unlockMutex(LavMutex *m) {
	((mutex*)m)->unlock();
	return Lav_ERROR_NONE;
}
