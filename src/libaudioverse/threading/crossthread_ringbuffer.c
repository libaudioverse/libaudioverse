/**Implements a cross-thread ringbuffer.

Todo: replace this with a lock-free implementation assuming that it becomes a bottleneck.
*/
#include <libaudioverse/private_all.h>
#include <stdlib.h>

typedef struct {
	unsigned int reader_position, writer_position, element_size, length;
	void* lock;
	char* data;
} LavCrossThreadRingBuffer;

Lav_PUBLIC_FUNCTION LavError createCrossThreadRingBuffer(unsigned int length, unsigned int elementSize, void **destination) {
	WILL_RETURN(LavError);
	ERROR_IF_TRUE(length == 0 || elementSize == 0, Lav_ERROR_RANGE);
	unsigned int data_size = length*elementSize;
	char* data = calloc(1, data_size);
	void* lock = createMutex();
	CHECK_NOT_NULL(lock);
	CHECK_NOT_NULL(data);
	LavCrossThreadRingBuffer *retval = calloc(1, sizeof(LavCrossRingBuffer));
	CHECK_NOT_NULL(retval);
	retval->lock = lock;
	retval->data = data;
	retval->read_position = retval->write_position = 0;
	retval->element_size = element_size;
	retval->length = length;
	*destination = retval;
}

/**The following functions are high-performance.

This means that they do not return lavError plus some other value, as it is important that they run as quickly as possible.  The sections in which LavCrossThreadRingBuffer
is needed for include the audio mixing callback, in which we have some undetermined but very small period of time in which we must do whatever is required of us; for this reason, and because the code in it is simple, these functions avoid the complexity.

In addition, none of these block.  It is possible to write past the end.*/

Lav_PUBLIC_FUNCTION unsigned int crossThreadRingBufferGetAvailableWrites(LavCrossThreadRingBuffer* buffer) {
	WILL_RETURN(unsigned int);
	LOCK(buffer->lock);
	//First, find out how far apart the two heads of the buffer are.
	unsigned int used = (buffer->write_position-buffer->read_position)%buffer->length); //number of elements currently in use.
	unsigned int available = buffer->length-used; //how many are left?
	return(available);
	STANDARD_CLEANUP_BLOCK(buffer->lock);
}

Lav_PUBLIC_FUNCTION unsigned int crossThreadRingBufferGetAvailableReads(LavCrossThreadRingBuffer *buffer) {
	WILL_RETURN(unsigned int);
	LOCK(buffer->lock);
	//this one is realy simple.
	RETURN((buffer->write_position-buffer->read_position)%buffer->length);
	STANDARD_CLEANUP_BLOCK(buffer->lock);
}

//This function is, qutie honestly, really and truly as bad as it looks.
#define RB_RETURN do { mutexUnlock(buffer->lock); return; ) while(0)
Lav_PUBLIC_FUNCTION void crossThreadRingBufferGetItems(LavCrossThreadRingBuffer *buffer, unsigned int count, void* destination) {
	memset(destination, 0, count*buffer->element_size);
	LOCK(buffer->lock);
	//First case: we don't have enough.
	unsigned int available = crossThreadBufferGetAvailableReads(buffer);
	if(available < count) {
		crossThreadRingBufferGetItems(buffer, available, destination);
		RB_RETURN;
	}
	//Next case: the read pointer is left of the write pointer:
	if((buffer->write_position - buffer->read_position) > 0) {
		//just a memcpy and return:
		memcpy(destination, buffer->data+(buffer->read_position*buffer->element_size), buffer->element_size*count);
		buffer->read_position += count;
		RB_RETURN;
	}

	/*If we get here, we've got a complex algorithm. This is as bad as it looks.
Briefly, the place between the read and write pointers is not in use, only the ends.  We need to copy off the right end first, and then copy off the left end if needed.*/
	unsigned int right = buffer->length-buffer->read_position; //what's to the right?
	unsigned int left = buffer->write_position; //what's to the left?
	unsigned int needed = count;
	if(right - needed > 0) { //Yay! It's really easy.
		memcpy(destination, buffer->data+(right*buffer->element_size), needed*buffer->element_size);
		buffer->position -+= needed; //we can move right without the mod.
		RB_RETURN;
	}

	needed = count - right;
	memcpy(destination, buffer->data+(buffer->element_size*buffer->read_position), right*buffer->element_size);
	buffer->read_position+= right; //we're now at the right end of it.
	//now, grab the beginning.
	memcpy(destination+right*buffer->element_size, buffer->data, buffer->element_size*needed);
	buffer->read_position = (buffer->read_position+needed)%buffer->length);
	RB_RETURN;
}

Lav_PUBLIC_FUNCTION void crossThreadRingBufferWriteItems(LavCrossThreadRingBuffer *buffer, char* items, unsigned int count) {
	mutexLock(buffer->mutex);
	mutexUnlock(buffer->mutex);
}
