/**Implements a cross-thread ringbuffer.

Todo: replace this with a lock-free implementation assuming that it becomes a bottleneck.
*/
#include <libaudioverse/private_all.h>
#include <stdlib.h>
#include <string.h>

//Apparently, C's mod is in fact not the discrete math operation.
//This function handles that.
int ringmod(int dividend, int divisor) {
	return dividend < 0 ? dividend%divisor+divisor : dividend%divisor;
}

Lav_PUBLIC_FUNCTION LavError createCrossThreadRingBuffer(int length, int elementSize, LavCrossThreadRingBuffer **destination) {
	WILL_RETURN(LavError);
	ERROR_IF_TRUE(length == 0 || elementSize == 0, Lav_ERROR_RANGE);
	int data_size = length*elementSize;
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
	RETURN(Lav_ERROR_NONE);
	BEGIN_CLEANUP_BLOCK
	DO_ACTUAL_RETURN;
}

/**The following functions are high-performance.

This means that they do not return lavError plus some other value, as it is important that they run as quickly as possible.  The sections in which LavCrossThreadRingBuffer
is needed for include the audio mixing callback, in which we have some undetermined but very small period of time in which we must do whatever is required of us; for this reason, and because the code in it is simple, these functions avoid the complexity.

In addition, none of these block.  It is possible to write past the end.*/

Lav_PUBLIC_FUNCTION int CTRBGetAvailableWrites(LavCrossThreadRingBuffer* buffer) {
	WILL_RETURN(int);
	LOCK(buffer->lock);
	//First, find out how far apart the two heads of the buffer are.
	int used = ringmod(buffer->write_position-buffer->read_position, buffer->length); //number of elements currently in use.
	int available = buffer->length-used; //how many are left?
	RETURN(available);
	STANDARD_CLEANUP_BLOCK(buffer->lock);
}

Lav_PUBLIC_FUNCTION int CTRBGetAvailableReads(LavCrossThreadRingBuffer *buffer) {
	WILL_RETURN(int);
	LOCK(buffer->lock);
	//this one is realy simple.
	RETURN(ringmod(buffer->write_position-buffer->read_position, buffer->length));
	STANDARD_CLEANUP_BLOCK(buffer->lock);
}

#define RB_RETURN do { mutexUnlock(buffer->lock); return; }  while(0)

Lav_PUBLIC_FUNCTION void CTRBGetItems(LavCrossThreadRingBuffer *buffer, int count, void* destination) {
	memset(destination, 0, count*buffer->element_size);
	mutexLock(buffer->lock);
	int available = CTRBGetAvailableReads(buffer);
	char* data = buffer->data;
	for(int i = 0; i < available && count > 0; i++, count--) {
		memcpy(destination, data+buffer->write_position*buffer->element_size, buffer->element_size);
		buffer->read_position++;
		buffer->read_position%=buffer->length;
	}
	RB_RETURN;
}

Lav_PUBLIC_FUNCTION void CTRBWriteItems(LavCrossThreadRingBuffer *buffer, int count, void* data) {
	mutexLock(buffer->lock);
	char* data_ptr = data;
	int available = CTRBGetAvailableWrites(buffer);
	for(int i = 0; i < available && count > 0; i++, count--) {
		memcpy(buffer->data+buffer->write_position*buffer->element_size, data_ptr, buffer->element_size);
		data_ptr += buffer->element_size;
		buffer->write_position++;
		buffer->write_position %= buffer->length;
	}
	RB_RETURN;
}
