"""Initializes nodes, checking their integrity.  That is, whether or not it is likely to crash when used."""
from make_cffi import *

def basic_node_checks(node):
	assert node is not ffi.NULL
	assert node.properties is not ffi.NULL
	for i in xrange(node.num_outputs):
		assert node.outputs[i].samples is not ffi.NULL
	for i in xrange(node.num_properties):
		prop = node.properties[i]
		assert prop.name is not ffi.NULL
		if prop.type == lav.Lav_PROPERTYTYPE_INT:
			assert prop.value.ival == prop.default_value.ival
		elif prop.type == lav.Lav_PROPERTYTYPE_FLOAT:
			assert prop.value.fval == prop.default_value.fval
		elif prop.type == lav.Lav_PROPERTYTYPE_DOUBLE:
			assert prop.value.dval == prop.default_value.dval
		elif prop.type == lav.Lav_PROPERTYTYPE_STRING:
			assert ffi.string(prop.value.sval) == ffi.string(prop.default_value.sval)
		assert prop.name is not ffi.NULL
	assert node.process is not ffi.NULL

def test_basic_node_integrity():
	"""Initializes the most basic type of node, and sees if it looks valid."""
	node = ffi.new("LavNode **")
	assert lav.Lav_ERROR_NONE == lav.Lav_makeNode(
		ffi.sizeof("LavNode"), #size
		5, #5 inputs.
		3, #3 outputs.
		3, #3 properties.
		lav.Lav_NODETYPE_ZEROS, #the type
		node)  #and the destination
	basic_node_checks(node[0])

def test_sine_node_integrity():
	node = ffi.new("LavNode **")
	assert lav.Lav_makeSineNode(node) == lav.Lav_ERROR_NONE
	basic_node_checks(node[0])
