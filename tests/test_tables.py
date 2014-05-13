"""Test tables.  This follows very similar logic to the sine wave test, but uses a 0, 1, 0, 1...pattern."""
from math import floor, ceil
from make_cffi import *

def test_create_table():
	table = ffi.new("LavTable**")
	assert lav.Lav_createTable(table) == lav.Lav_ERROR_NONE
	assert table is not ffi.NULL and table[0] is not ffi.NULL

