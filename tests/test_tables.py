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
