from make_cffi import *
from math import *

def test_sine():
	"""This deserves explanation.
There is almost no way in which we can actually match a sine wave's output reliably from run to run or computer to computer.
We can be pretty sure, however, that the calculations done here and the calculations done in trhe C library are very, very close."""
	accuracy = 0.01
	n1, n2 = ffi.new("LavNode **"), ffi.new("LavNode **")
	assert lav.Lav_createSineNode(n1) == lav.Lav_ERROR_NONE
	assert lav.Lav_createNode(1, 0, 0, lav.Lav_NODETYPE_ZEROS, n2) == lav.Lav_ERROR_NONE
	n1, n2 = n1[0], n2[0]
	sr = n1.sr
	time_delta = 1/float(sr)
	freq = ffi.new("float*")
	lav.Lav_getFloatProperty(n1, lav.Lav_SINE_FREQUENCY, freq)
	freq = freq[0]
	verified_output = [sin(i*time_delta*freq*pi*2) for i in xrange(512)]
	arr = ffi.new("float[]", len(verified_output))
	arr_ptr = ffi.cast("float*", arr)
	assert lav.Lav_setParent(n2, n1, 0, 0) == lav.Lav_ERROR_NONE
	#do the read.
	assert lav.Lav_streamReadSamples(ffi.addressof(n2.inputs[0]), len(verified_output), arr_ptr) == lav.Lav_ERROR_NONE
	for i, val in enumerate(verified_output):
		assert abs(arr[i]-val) < accuracy
