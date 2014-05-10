#include <libaudioverse/private_all.h>
#include <portaudio.h>
#include <uthash.h>

typedef struct {
	void* thread_handle;
	LavGraph *graph;
	PaStream *stream;
	void* initial_startup_mutex; //needed so threads can know about themselves.
} ThreadParams;

void audioOutputThread(void* vparam);

unsigned int thread_counter = 0; //Used for portaudio init and deinit.  When decremented to 0, deinit portaudio.

LavError createAudioOutputThread(LavGraph *graph, void **destination) {
	WILL_RETURN(LavError);
	if(thread_counter == 0) {
		PaError e = Pa_Initialize();
		ERROR_IF_TRUE(e != paNoError, Lav_ERROR_CANNOT_INIT_AUDIO);
	}
	thread_counter ++;
	//let's try to get a stream.
	PaStream *stream;
	ERROR_IF_TRUE(Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, SR, 128, NULL, NULL) != paNoError, Lav_ERROR_CANNOT_INIT_AUDIO);
	CHECK_NOT_NULL(graph);
	LOCK(graph->mutex);
	ThreadParams *param = calloc(1, sizeof(ThreadParams));
	param->graph = graph;
	param->stream = stream;
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
	PaStream *stream = param->stream;
}
