"""Test tables.  This follows very similar logic to the sine wave test, but uses a 0, 1, 0, 1...pattern."""
from math import floor, ceil
from make_cffi import *

delta = 0.1
iterations = 2
accuracy = 0.00001

def make_outputs():
	output1 = [i%2 for i in xrange(100)]
	output2 = []
	pos = 0.0
	while pos < len(output1)*iterations-delta: #-detal doesn't include the endpoint, like xrange.
		samp1, samp2 = floor(pos), ceil(pos)
		samp1%=len(output1)
		samp2%=len(output1)
		weight2, weight1 = pos-floor(pos), ceil(pos)-pos
		output2.append(weight1*output1[int(samp1)]+weight2*output1[int(samp2)])
		pos+=delta
	return output1, output2

def test_create_table():
	table = ffi.new("LavTable**")
	assert lav.Lav_createTable(table) == lav.Lav_ERROR_NONE
	assert table is not ffi.NULL and table[0] is not ffi.NULL

def test_compute_samples():
	original, accepted = make_outputs()
	table = ffi.new("LavTable**")
	assert lav.Lav_createTable(table) == lav.Lav_ERROR_NONE
	table = table[0]
	assert table is not ffi.NULL
	#now, we submit the sample buffer.
	duration = len(original) #put the samples on  one-second boundaries
	assert lav.Lav_tableSetSamples(table, len(original), duration, original) == lav.Lav_ERROR_NONE
	output = ffi.new("float[" + str(len(original)*iterations*int(1/delta)) + "]")
	start, end = 0, delta*len(original)
	output_ptr = output
	for i in xrange(iterations):
		assert lav.Lav_tableComputeSampleRange(table, start, end, abs(int((end-start)/delta)), output_ptr) == lav.Lav_ERROR_NONE
		output_ptr += int((end-start)/delta)
		start = end+delta
		end*=2
	assert len(output) == len(accepted)
	for i, j in zip(enumerate(output), enumerate(accepted)):
		iaccepted = j[1]
		ioutput = i[1]
		output_counter = i[0]
		accepted_counter = j[0]
		assert abs(ioutput-iaccepted) < accuracy
