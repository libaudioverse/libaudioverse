#include <libaudioverse/private_all.h>
#include <portaudio.h>
#include <uthash.h>

typedef struct {
	void* thread_handle;
	LavGraph *graph;
	void* initial_startup_mutex; //needed so threads can know about themselves.
} ThreadParams;

void audioOutputThread(void* vparam);

LavError createAudioOutputThread(LavGraph *graph, void **destination) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(graph);
	LOCK(graph->mutex);
	ThreadParams *param = calloc(1, sizeof(ThreadParams));
	param->graph = graph;
	LavError err;
	err = createMutex(&(param->initial_startup_mutex));
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	err = mutexLock(param->initial_startup_mutex);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	void* th;
	err = threadRun(audioOutputThread, param, &th);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	param->thread_handle = th;
	//let it go!
	err = mutexUnlock(param->initial_startup_mutex);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	STANDARD_CLEANUP_BLOCK(graph->mutex);
}

void audioOutputThread(void* vparam) {
	ThreadParams *param = (ThreadParams*)vparam;
	LavGraph *graph = param->graph;
	void* threadHandle = param->thread_handle;
}
