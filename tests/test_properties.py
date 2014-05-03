"""Makes a node with a property of each type, and then tests getting, setting, and clearing."""
from make_cffi import *

def property_node():
	node = ffi.new("LavNode **")
	assert lav.Lav_makeNode(1, 1, 4, lav.Lav_NODETYPE_ZEROS, node) == lav.Lav_ERROR_NONE
	node[0].properties[0].type = lav.Lav_PROPERTYTYPE_INT
	node[0].properties[1].type = lav.Lav_PROPERTYTYPE_FLOAT
	node[0].properties[2].type = lav.Lav_PROPERTYTYPE_DOUBLE
	node[0].properties[3].type = lav.Lav_PROPERTYTYPE_STRING
	return node

def test_property_getters():
	ival = ffi.new("int *")
	fval = ffi.new("float *")
	dval = ffi.new("double *")
	sval = ffi.new("char **")
	node = property_node()
	assert lav.Lav_getIntProperty(node[0], 0, ival) == lav.Lav_ERROR_NONE
	assert lav.Lav_getFloatProperty(node[0], 1, fval) == lav.Lav_ERROR_NONE
	assert lav.Lav_getDoubleProperty(node[0], 2, dval) == lav.Lav_ERROR_NONE
	assert lav.Lav_getStringProperty(node[0], 3, sval) == lav.Lav_ERROR_NONE
