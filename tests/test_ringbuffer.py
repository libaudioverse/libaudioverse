import random
from make_cffi import *

def test_ringbuffer():
	"""Test the ringbuffer, extensively.
We make a list of 1000 ints, and pass it through the ringbuffer 5 times.  In each case, we should *always* get the same back out."""
	expected = [random.randint(0, 500) for i in xrange(1000)]
	cffi_array = ffi.new("unsigned int[" + str(len(expected)) +"]", expected)
	size = ffi.sizeof("unsigned int")
	rb = ffi.new("LavCrossThreadRingBuffer **")
	lav.createCrossThreadRingBuffer(5, size, rb)
	rb = rb[0]
