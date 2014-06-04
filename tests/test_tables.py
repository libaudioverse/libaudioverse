"""Test tables.  This follows very similar logic to the sine wave test, but uses a 0, 1, 0, 1...pattern."""
from math import floor, ceil
from make_cffi import *
from .conf import *

def test_create_table():
	assert lav.Lav_initializeLibrary() == lav.Lav_ERROR_NONE
	table = ffi.new("LavTable**")
	assert lav.Lav_createTable(table) == lav.Lav_ERROR_NONE
	assert table is not ffi.NULL and table[0] is not ffi.NULL

def test_table_clear():
	assert lav.Lav_initializeLibrary() == lav.Lav_ERROR_NONE
	table = ffi.new("LavTable **")
	assert lav.Lav_createTable(table) == lav.Lav_ERROR_NONE
	table = table[0]
	#An empty table is going to always yield 0, for any index.
	sample = ffi.new("float[1]")
	zero_indices = [i/10.0 for i in xrange(100)]
	for i in zero_indices:
		assert lav.Lav_tableGetSample(table, i, sample) == lav.Lav_ERROR_NONE
		assert sample[0] < accuracy

#some values for a table.
original = [0, 1] #A triangle wave.
#correct values for the table used in these tests.
halves= [0, 0.5, 1, 0.5]
fourths = [0, 0.25, 0.5, 0.75, 1, 0.75, 0.5, 0.25]
eighths = [0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875, 1, 0.875, 0.75, 0.625, 0.5, 0.375, 0.25, 0.125]
correct = [halves, fourths, eighths]

def make_table():
	global original
	table = ffi.new("LavTable **")
	assert lav.Lav_createTable(table) == lav.Lav_ERROR_NONE
	table = table[0]
	assert lav.Lav_tableSetSamples(table, len(original), original) == lav.Lav_ERROR_NONE
	return table

def test_table_get_sample():
	assert lav.Lav_initializeLibrary() == lav.Lav_ERROR_NONE
	global halves, fourths, eighths, correct, original, accuracy
	table = make_table()
	#test 1: reading exactly on integer boundaries should always produce the same value.  I like 500.
	for i in xrange(500):
		original_index = i%len(original)
		sample = ffi.new("float[1]")
		lav.Lav_tableGetSample(table, i, sample)
		assert abs(sample[0]-original[original_index]) < accuracy
	#test 2: Reading at fractional values, with this setup, is predictable.
	for expected, denom  in zip(correct, (2,4,8)):
		for i in xrange(len(expected)):
			index = i/float(denom)
			correct_index = i%len(expected) #index of the correct answer.
			sample = ffi.new("float[1]")
			assert lav.Lav_tableGetSample(table, index, sample) == lav.Lav_ERROR_NONE
			assert abs(sample[0]-expected[correct_index]) < accuracy

def test_table_get_sample_range():
	assert lav.Lav_initializeLibrary() == lav.Lav_ERROR_NONE
	global halves, fourths, eighths, original, accuracy
	table = make_table()
	outarray = ffi.new("float[" + str(len(eighths)) + "]")
	assert lav.Lav_tableGetSamples(table, 0, 1, len(original), outarray) == lav.Lav_ERROR_NONE
	for i in xrange(len(original)): assert abs(outarray[i]-original[i]) < accuracy
	assert lav.Lav_tableGetSamples(table, 0, 0.5, len(halves), outarray) == lav.Lav_ERROR_NONE
	for i in xrange(len(halves)): assert abs(outarray[i]-halves[i]) < accuracy
	assert lav.Lav_tableGetSamples(table, 0, 0.25, len(fourths), outarray) == lav.Lav_ERROR_NONE
	for i in xrange(len(fourths)): assert abs(outarray[i]-fourths[i]) < accuracy
	assert lav.Lav_tableGetSamples(table, 0, 1/8.0, len(eighths), outarray) == lav.Lav_ERROR_NONE
	for i in xrange(len(eighths)): assert abs(outarray[i]-eighths[i]) < accuracy
