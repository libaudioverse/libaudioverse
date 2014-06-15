/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Implements a cross-thread ringbuffer.

Todo: replace this with a lock-free implementation assuming that it becomes a bottleneck.
*/
#include <libaudioverse/private_all.h>
#include <stdlib.h>
#include <string.h>
const int READ_OP = -1;
const int WRITE_OP = 1;



Lav_PUBLIC_FUNCTION LavError createCrossThreadRingBuffer(int length, int elementSize, LavCrossThreadRingBuffer **destination) {
	STANDARD_PREAMBLE;
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
	retval->last_op = READ_OP;
	*destination = retval;
	SAFERETURN(Lav_ERROR_NONE);
	BEGIN_CLEANUP_BLOCK
	DO_ACTUAL_RETURN;
}

/**The following functions are high-performance.

This means that they do not return lavError plus some other value, as it is important that they run as quickly as possible.  The sections in which LavCrossThreadRingBuffer
is needed for include the audio mixing callback, in which we have some undetermined but very small period of time in which we must do whatever is required of us; for this reason, and because the code in it is simple, these functions avoid the complexity.

In addition, none of these block.  It is possible to write past the end.*/

Lav_PUBLIC_FUNCTION int CTRBGetAvailableWrites(LavCrossThreadRingBuffer* buffer) {
	mutexLock(buffer->lock);
	//First, find out how far apart the two heads of the buffer are.
	int used = ringmodi(buffer->write_position-buffer->read_position, buffer->length); //number of elements currently in use.
	int available = buffer->length-used; //how many are left?
	if(available == 0 && buffer->last_op == READ_OP) available = buffer->length;
	mutexUnlock(buffer->lock);
	return available;
}

Lav_PUBLIC_FUNCTION int CTRBGetAvailableReads(LavCrossThreadRingBuffer *buffer) {
	mutexLock(buffer->lock);
	//this one is realy simple.
	int available = ringmodi(buffer->write_position-buffer->read_position, buffer->length);
	if(available == 0 && buffer->last_op == WRITE_OP) available = buffer->length;
	mutexUnlock(buffer->lock);
	return available;
}

#define RB_RETURN do { mutexUnlock(buffer->lock); return; }  while(0)

Lav_PUBLIC_FUNCTION void CTRBGetItems(LavCrossThreadRingBuffer *buffer, int count, void* destination) {
	char *dest = destination; //gives this a size.
	memset(dest, 0, count*buffer->element_size);
	mutexLock(buffer->lock);
	int available = CTRBGetAvailableReads(buffer);
	char* data = buffer->data;
	for(int i = 0; i < available && count > 0; i++, count--) {
		memcpy(dest, data+buffer->read_position*buffer->element_size, buffer->element_size);
		dest += buffer->element_size;
		buffer->read_position++;
		buffer->read_position%=buffer->length;
	}
	buffer->last_op = READ_OP;
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
	buffer->last_op = WRITE_OP;
	RB_RETURN;
}
