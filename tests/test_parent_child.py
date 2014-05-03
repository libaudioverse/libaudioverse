"""Tests the parent-child relationships."""
from make_cffi import *

def test_parent_child():
	node1, node2 = ffi.new("LavNode **"), ffi.new("LavNode **")
	lav.Lav_makeNode(0, 1, 0, lav.Lav_NODETYPE_ZEROS, node1)
	lav.Lav_makeNode(1, 0, 0, lav.Lav_NODETYPE_ZEROS, node2)
	node1, node2 = node1[0], node2[0]
	assert node1 is not ffi.NULL and node2 is not ffi.NULL
	#the following let us pull out info.
	parent = ffi.new("LavNode **")
	slot = ffi.new("unsigned int*")
	assert lav.Lav_setParent(node2, node1, 0, 0) == lav.Lav_ERROR_NONE
	assert lav.Lav_getParent(node2, 0, parent, slot) == lav.Lav_ERROR_NONE
	assert parent[0] == node1 and slot[0] == 0
