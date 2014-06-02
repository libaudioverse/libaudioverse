"""Makes a node with a property of each type, and then tests getting, setting, and clearing."""
from make_cffi import *
import itertools

keep_forever = set() #cffi frees memory when garbage collection happens; this prevents it.

def property_node():
	graph = ffi.new("LavGraph **")
	node = ffi.new("LavNode **")
	assert lav.Lav_createGraph(44100, 128, graph) == lav.Lav_ERROR_NONE
	assert lav.Lav_createNode(1, 1, lav.Lav_NODETYPE_ZEROS, graph[0], node) == lav.Lav_ERROR_NONE
	property_array = ffi.new("LavProperty[4]")
	keep_forever.add(property_array)
	properties = ffi.new("LavProperty*[4]", [property_array, property_array+1, property_array+2, property_array+3])
	keep_forever.add(properties)
	node[0].properties = properties
	node[0].num_properties = 4
	node[0].properties[0].type = lav.Lav_PROPERTYTYPE_INT
	node[0].properties[1].type = lav.Lav_PROPERTYTYPE_FLOAT
	node[0].properties[2].type = lav.Lav_PROPERTYTYPE_DOUBLE
	node[0].properties[3].type = lav.Lav_PROPERTYTYPE_STRING
	return graph, node

def test_property_getters():
	ival = ffi.new("int *")
	fval = ffi.new("float *")
	dval = ffi.new("double *")
	sval = ffi.new("char **")
	graph, node = property_node()
	assert lav.Lav_getIntProperty(node[0], 0, ival) == lav.Lav_ERROR_NONE
	assert lav.Lav_getFloatProperty(node[0], 1, fval) == lav.Lav_ERROR_NONE
	assert lav.Lav_getDoubleProperty(node[0], 2, dval) == lav.Lav_ERROR_NONE
	assert lav.Lav_getStringProperty(node[0], 3, sval) == lav.Lav_ERROR_NONE
	eronious_combinations = set(itertools.product(
		(0, 1, 2, 3),
		zip((lav.Lav_getIntProperty, lav.Lav_getFloatProperty, lav.Lav_getDoubleProperty, lav.Lav_getStringProperty),
		(ival, fval, dval, sval))))
	eronious_combinations.remove((0, (lav.Lav_getIntProperty, ival)))
	eronious_combinations.remove((1, (lav.Lav_getFloatProperty, fval)))
	eronious_combinations.remove((2, (lav.Lav_getDoubleProperty, dval)))
	eronious_combinations.remove((3, (lav.Lav_getStringProperty, sval)))
	#eronious_combinations contains only property, function, variable associations that will error with lav_ERROR_TYPE_MISMATCH
	for prop, func_dest_pair in eronious_combinations:
		func, dest = func_dest_pair
		assert func(node[0], prop, dest) == lav.Lav_ERROR_TYPE_MISMATCH
