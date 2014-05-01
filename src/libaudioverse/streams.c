/**There are two concepts in this file, streams and buffers.

A node writes to a buffer.  A node reads from a stream.  Streams have associated buffers and positions, such that more than one stream can use one buffer.  no node may know about streams connected to it.

These concepts are incredibly tightly intertwined, and are thus included together here.  Bugs in one are bugs in the other, and implementation changes require touching both.*/

#include <libaudioverse/libaudioverse.h>
#include "private_macros.h"
#include <stdlib.h>
#include <string.h> //some functions here are useful for things that aren't strings.

