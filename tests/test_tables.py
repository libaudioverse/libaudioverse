"""Test tables.  This follows very similar logic to the sine wave test, but uses a 0, 1, 0, 1...pattern."""
from math import floor, ceil
from make_cffi import *
accuracy = 1e-3

def test_create_table():
	table = ffi.new("LavTable**")
	assert lav.Lav_createTable(table) == lav.Lav_ERROR_NONE
	assert table is not ffi.NULL and table[0] is not ffi.NULL

def test_table_clear():
	table = ffi.new("LavTable **")
	assert lav.Lav_createTable(table) == lav.Lav_ERROR_NONE
	table = table[0]
	#An empty table is going to always yield 0, for any index.
	sample = ffi.new("float[1]")
	zero_indices = [i/10.0 for i in xrange(100)]
	for i in zero_indices:
		assert lav.Lav_tableGetSample(table, i, sample) == lav.Lav_ERROR_NONE
		assert sample[0] < accuracy

def test_table_get_sample():
	original = [0, 1] #A triangle wave.
	table = ffi.new("LavTable **")
	assert lav.Lav_createTable(table) == lav.Lav_ERROR_NONE
	table = table[0]
	assert lav.Lav_tableSetSamples(table, len(original), original) == lav.Lav_ERROR_NONE
	#test 1: reading exactly on integer boundaries should always produce the same value.  I like 500.
	for i in xrange(500):
		original_index = i%len(original)
		sample = ffi.new("float[1]")
		lav.Lav_tableGetSample(table, i, sample)
		assert abs(sample[0]-original[original_index]) < accuracy
	#test 2: Reading at fractional values, with this setup, is predictable.
	halves= [0, 0.5, 1, 0.5]
	fourths = [0, 0.25, 0.5, 0.75, 1, 0.75, 0.5, 0.25]
	eighths = [0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875, 1, 0.875, 0.75, 0.625, 0.5, 0.375, 0.25, 0.125]
	correct = [halves, fourths, eighths]
	for expected, denom  in zip(correct, (2,4,8)):
		for i in xrange(len(expected)):
			index = i/float(denom)
			correct_index = i%len(expected) #index of the correct answer.
			sample = ffi.new("float[1]")
			assert lav.Lav_tableGetSample(table, index, sample) == lav.Lav_ERROR_NONE
			assert abs(sample[0]-expected[correct_index]) < accuracy
