/**Implements a cross-thread ringbuffer.

Todo: replace this with a lock-free implementation assuming that it becomes a bottleneck.
*/
#include <libaudioverse/private_all.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	unsigned int read_position, write_position, element_size, length;
	void* lock;
	char* data;
} LavCrossThreadRingBuffer;

Lav_PUBLIC_FUNCTION LavError createCrossThreadRingBuffer(unsigned int length, unsigned int elementSize, void **destination) {
	WILL_RETURN(LavError);
	ERROR_IF_TRUE(length == 0 || elementSize == 0, Lav_ERROR_RANGE);
	unsigned int data_size = length*elementSize;
	char* data = calloc(1, data_size);
	void* lock;
	createMutex(&lock);
	CHECK_NOT_NULL(lock);
	CHECK_NOT_NULL(data);
	LavCrossThreadRingBuffer *retval = calloc(1, sizeof(LavCrossThreadRingBuffer));
	CHECK_NOT_NULL(retval);
	retval->lock = lock;
	retval->data = data;
	retval->read_position = retval->write_position = 0;
	retval->element_size = elementSize;
	retval->length = length;
	*destination = retval;
	BEGIN_CLEANUP_BLOCK
	DO_ACTUAL_RETURN;
}

/**The following functions are high-performance.

This means that they do not return lavError plus some other value, as it is important that they run as quickly as possible.  The sections in which LavCrossThreadRingBuffer
is needed for include the audio mixing callback, in which we have some undetermined but very small period of time in which we must do whatever is required of us; for this reason, and because the code in it is simple, these functions avoid the complexity.

In addition, none of these block.  It is possible to write past the end.*/

Lav_PUBLIC_FUNCTION unsigned int crossThreadRingBufferGetAvailableWrites(LavCrossThreadRingBuffer* buffer) {
	WILL_RETURN(unsigned int);
	LOCK(buffer->lock);
	//First, find out how far apart the two heads of the buffer are.
	unsigned int used = (buffer->write_position-buffer->read_position)%buffer->length; //number of elements currently in use.
	unsigned int available = buffer->length-used; //how many are left?
	RETURN(available);
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
#define RB_RETURN do { mutexUnlock(buffer->lock); return; }  while(0)
Lav_PUBLIC_FUNCTION void crossThreadRingBufferGetItems(LavCrossThreadRingBuffer *buffer, unsigned int count, void* destination) {
	memset(destination, 0, count*buffer->element_size);
	mutexLock(buffer->lock);
	unsigned int available = crossThreadRingBufferGetAvailableReads(buffer);
	char* data = buffer->data;
	for(unsigned int i = 0; i < available && count > 0; i++, count--) {
		memcpy(destination, data+buffer->write_position*buffer->element_size, buffer->element_size);
		buffer->read_position++;
		buffer->read_position%=buffer->length;
	}
	RB_RETURN;
}

Lav_PUBLIC_FUNCTION void crossThreadRingBufferWriteItems(LavCrossThreadRingBuffer *buffer, unsigned int count, void* data) {
	mutexLock(buffer->lock);
	char* data_ptr = data;
	unsigned int available = crossThreadRingBufferGetAvailableWrites(buffer);
	for(unsigned int i = 0; i < available && count > 0; i++, count--) {
		memcpy(buffer->data+buffer->write_position*buffer->element_size, data_ptr, buffer->element_size);
		data_ptr += buffer->element_size;
	}
	RB_RETURN;
}
