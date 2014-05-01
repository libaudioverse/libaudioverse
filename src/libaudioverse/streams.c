/**There are two concepts in this file, streams and buffers.

A node writes to a buffer.  A node reads from a stream.  Streams have associated buffers and positions, such that more than one stream can use one buffer.  No node may know about streams connected to it.

These concepts are incredibly tightly intertwined, and are thus included together here.  Bugs in one are bugs in the other, and implementation changes require touching both.

The public functions will always work, but it is worth noting that the current implementation is the simplest possible: if a writer writes faster than a reader, the reader will skip values.  If there is more than one reader, and one of them causes additional processing, values may also be skipped.

Before reading this file, know the following mod trick:
3 mod 10 is 3, -3 mod 10 is 7.
So if the reader is at 2, the writer is at 5, and the buffer is 10 ling:
(5-2) mod 10 is 3.
But suppose the reader is 5 and the writer is 2, that is the writer just went past the end:
(2-5) mod 10 = -3 mod 10 = 7.
*/

#include <libaudioverse/libaudioverse.h>
#include "private_macros.h"
#include <stdlib.h>
#include <string.h> //some functions here are useful for things that aren't strings.

/**First, buffers.

Note that we can only write to a buffer, at least without a stream.  Also, all buffers start with one sample written to them: a single 0.  This is part of allowing cyclic graphs, which is needed for reverb.*/

Lav_PUBLIC_FUNCTION LavError Lav_bufferWriteSample(LavSampleBuffer *buffer, float sample) {
	//calculate the next position.
	unsigned int next_position = (buffer->write_position+1) % buffer->length; //wrap if necessary.
	buffer->samples[next_position] = sample;
	buffer->write_position = next_position;
	return Lav_ERROR_NONE;
}

/**Now, streams.*/
Lav_PUBLIC_FUNCTION Lav_streamReadSamples(LavStream *stream, unsigned int count, float *destination) {
	CHECK_NOT_NULL(stream);
	if(stream->associated_buffer == NULL) { //the best we can do is 0s.
		for(unsigned int i = 0; i < count; i++) {
			destination[i] = 0;
		}
	}
	else { //we do have a buffer and we can read from it.
		for(unsigned int i = 0; i < count; i++) {
			destination[i] = stream->associated_buffer->samples[stream->position]; //read from our current position; and
			stream->position = (stream->position+1)%stream->associated_buffer->length; //figure out our next position.
			//If we are now at the position of the write head, we need to get more audio.
			if(stream->position == stream->associated_buffer->write_position) stream->associated_buffer->owner.node->process(
				stream->associated_buffer->owner.node, 1); //ask for a sample.
		}
	}
	return Lav_ERROR_NONE;
}
