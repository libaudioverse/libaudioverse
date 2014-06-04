import random
from make_cffi import *

def test_ringbuffer():
	"""Test the ringbuffer, extensively.
We make a list of 1000 ints, and pass it through the ringbuffer 4 times.  In each case, we should *always* get the same back out.
We skip 3 because it complicates the logic, and the others should be good enough."""
	assert lav.Lav_initializeLibrary() == lav.Lav_ERROR_NONE
	expected = [random.randint(0, 500) for i in xrange(1000)]
	cffi_array = ffi.new("unsigned int[" + str(len(expected)) +"]", expected)
	size = ffi.sizeof("unsigned int")
	rb = ffi.new("LavCrossThreadRingBuffer **")
	assert lav.Lav_ERROR_NONE == lav.createCrossThreadRingBuffer(5, size, rb)
	rb = rb[0]
	for i in [1, 2, 4, 5]:
		arr = ffi.new("unsigned int[" + str(i) + "]")
		l = []
		for j in xrange(0, 1000, i):
			#write i items into the ring buffer.
			writing = ffi.new("unsigned int [" + str(i) + "]", expected[j:j+i])
			lav.CTRBWriteItems(rb, i, writing)
			assert lav.CTRBGetAvailableReads(rb) == i
			lav.CTRBGetItems(rb, i, arr)
			l += list(arr)
		assert l == expected