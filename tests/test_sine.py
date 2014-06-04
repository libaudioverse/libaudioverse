from make_cffi import *
from math import *
from .conf import *

def test_sine():
	"""This deserves explanation.
There is almost no way in which we can actually match a sine wave's output reliably from run to run or computer to computer.
We can be pretty sure, however, that the calculations done here and the calculations done in the C library are very, very close."""
	assert lav.Lav_initializeLibrary() == lav.Lav_ERROR_NONE
	graph = ffi.new("LavGraph **")
	n1 = ffi.new("LavNode **")
	assert lav.Lav_createGraph(44100, 512, graph) == lav.Lav_ERROR_NONE
	assert lav.Lav_createSineNode(graph[0], n1) == lav.Lav_ERROR_NONE
	assert lav.Lav_graphSetOutputNode(graph[0], n1[0]) == lav.Lav_ERROR_NONE
	n1= n1[0]
	sr = n1.graph.sr
	time_delta = 1/float(sr)
	freq = ffi.new("float*")
	lav.Lav_getFloatProperty(n1, lav.Lav_SINE_FREQUENCY, freq)
	freq = freq[0]
	verified_output = [sin(i*time_delta*freq*pi*2) for i in xrange(512)]
	arr = ffi.new("float[]", len(verified_output))
	arr_ptr = ffi.cast("float*", arr)
	#do the read.
	assert lav.Lav_graphReadAllOutputs(graph[0], arr_ptr) == lav.Lav_ERROR_NONE
	for i, val in enumerate(verified_output):
		assert abs(arr[i]-val) < accuracy
