"""Initializes nodes, checking their integrity.  That is, whether or not it is likely to crash when used."""
from make_cffi import *
from .conf import *

def basic_node_checks(node):
	assert node is not ffi.NULL
	assert node.properties is not ffi.NULL
	assert ffi.cast("LavNode*", node).graph is not ffi.NULL
	for i in xrange(node.num_outputs):
		assert node.outputs[i] is not ffi.NULL
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
	assert lav.Lav_initializeLibrary() == lav.Lav_ERROR_NONE
	graph = ffi.new("LavObject **")
	node = ffi.new("LavObject **")
	assert lav.Lav_ERROR_NONE == lav.Lav_createGraph(44100, 128, graph)
	assert lav.Lav_ERROR_NONE == lav.Lav_createNode(
		5, #5 inputs.
		3, #3 outputs.
		lav.Lav_NODETYPE_ZEROS, #the type
		graph[0], #the graph
		node)  #and the destination
	basic_node_checks(node[0])

def test_sine_node_integrity():
	assert lav.Lav_initializeLibrary() == lav.Lav_ERROR_NONE
	graph = ffi.new("LavObject **")
	node = ffi.new("LavObject **")
	assert lav.Lav_ERROR_NONE == lav.Lav_createGraph(44100, 128, graph)
	assert lav.Lav_createSineNode(graph[0], node) == lav.Lav_ERROR_NONE
	basic_node_checks(node[0])
