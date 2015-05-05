"""Extracts nodes of interest from a pycparser parse of bindings.h run through a platform-specific preprocessor and compares it with the metadata in metadata.y.

The information extracted by this module is placed in all_info, a dict with the following keys.
functions: A set of function instances. Keys are the names.
typedefs: A set of type instances describing the final form of typedefs.
constants: An extracted list of all constants.  This is a flat representation computed by reading all enumerations.
constants_by_enum:Same as constants, but grouped by the enumeration and placed in dictionaries; keys are enumeration names.
important_enums: The enums which metadata marks as important in some way.
metadata: The parsed yaml document itself as a dict.
"""



from pycparser import *
#we need to be able to compare with isinstance, unfortunately. Grab all of these too.
from pycparser.c_ast import *
import subprocess
import sys
import os.path
from collections import OrderedDict
import yaml
from . import metadata_handler

#this is a helper class representing a type.
#base is int, etc.
#indirection is the number of *s. int* is 1, etc.
class TypeInfo(object):
	def __init__(self, base, indirection, typedef_from = None):
		self.base = base
		self.indirection = indirection

#helper class for functions: return_type, args, name. Return_type and args should be TypeInfos.
class FunctionInfo(object):
	def __init__(self, return_type, name, args):
		self.return_type = return_type
		self.name = name
		self.args = tuple(args) #forcing this is a really good idea.
		self.input_args = tuple([i for i in args if 'destination' not in i.name])
		self.output_args = tuple([i for i in args if 'destination' in i.name])

#parameter.
class ParameterInfo(object):
	def __init__(self, type, name):
		self.type = type
		self.name = name

def get_root_directory():
	return os.path.split(os.path.split(os.path.abspath(__file__))[0])[0]

def make_ast():
	#compute the input file.
	#this gives us the root directory of the repository.
	root_directory = get_root_directory()
	input_file = os.path.join(root_directory, 'include', 'libaudioverse', 'binding.h')

	if sys.platform == 'win32':
		command = 'cl'
		args = '/nologo /EP ' + input_file
	else:
		command  = 'cpp'
		args = input_file

	text = subprocess.check_output(command + ' ' + args, shell = True)
	#convert from windows to linux newlines, if needed.
	text = text.replace('\r\n', '\n')

	#build a cffi parser.
	parser = c_parser.CParser()
	ast = parser.parse(text)
	return ast

def extract_enums(ast):
	"""Returns a dict of enum constant names to their values as integers."""
	#we don't allow declaring nested types, so we know that all enums are at the top level-that is, they are to be found in ast.ext.
	enum_list = [i.type for i in ast.ext if isinstance(i, Decl) and isinstance(i.type, Enum)]
	#now, we note that we can find--for a specific enum e--:
	#name = e.name, values are in e.values as e.values[index].value and names in e.values[index]name
	#note that minimal interpretation is needed so that we can have negated constants-pycparser is for interpreters, not this, and so represents them as a unary minus in the ast.
	constants_by_enum = OrderedDict()
	for enum in enum_list:
		implicit_value = 0
		constants_by_enum[enum.name] = dict()
		for enum_value in enum.values.enumerators:
			val = enum_value.value
			if val is None:
				constants_by_enum[enum.name][enum_value.name]=implicit_value
			elif isinstance(val, Constant):
				implicit_value = int(enum_value.value.value)
				constants_by_enum[enum.name][enum_value.name] = implicit_value
			elif isinstance(val, UnaryOp) and val.op == '-':
				implicit_value = int('-' + val.expr.value)
				constants_by_enum[enum.name][enum_value.name] = implicit_value
			implicit_value+=1
	return constants_by_enum

def extract_typedefs(ast):
	"""Returns a dict of typedefs.  Keys are names, values are TypeInfos describing the type."""
	#again, we expect them at the top levle--if they're not, we'll miss them.
	#the primary use of this is a bit later, when we build function objects-we aggregate typedefs when possible.
	typedef_list = [i for i in ast.ext if isinstance(i, Typedef)]
	typedefs = OrderedDict()
	for typedef in typedef_list:
		name = typedef.name
		typedefs[name] = compute_type_info(node = typedef, typedefs =typedefs)
	return typedefs

def compute_type_info(node, typedefs):
	indirection = 0
	currently_examining = node.type
	while isinstance(currently_examining, PtrDecl):
		indirection += 1
		currently_examining = currently_examining.type
	if isinstance(currently_examining, TypeDecl):
		currently_examining  = currently_examining.type
		name = " ".join(currently_examining.names)
		#first, make a TypeInfo
		info = TypeInfo(base = name, indirection = indirection)
		return info
	elif isinstance(currently_examining, FuncDecl):
		base = compute_function_info(func = currently_examining, typedefs =typedefs)
		return TypeInfo(base = base, indirection = indirection)

def compute_function_info(func, typedefs, name = ""):
	return_type = compute_type_info(node = func, typedefs = typedefs) #not func.type-the function expects one node above.
	if func.args is not None:
		types = [compute_type_info(node = i, typedefs = typedefs) for i in func.args.params]
		names = [i.name for i in func.args.params]
		args = zip(types, names)
		args = tuple([ParameterInfo(i[0], i[1]) for i in args])
	else:
		args = ()
	return FunctionInfo(return_type, name, args)

def extract_functions(ast, typedefs):
	function_list = [i for i in ast.ext if isinstance(i, Decl) and isinstance(i.type, FuncDecl)]
	functions = OrderedDict()
	for function in function_list:
		name = function.name
		functions[name] = compute_function_info(func = function.type, typedefs =typedefs, name= name)
	return functions

def get_all_info():
	ast=make_ast()
	constants_by_enum = extract_enums(ast=ast)
	constants = dict()
	for i in constants_by_enum.values():
		constants.update(i)

	#remove anything that ends in _MAX from constants_by_enum at this point.
	#rationale: the _MAX constants are needed in very specific places, but not by code that auto-binds enums.
	for i in constants_by_enum.itervalues():
		for j in dict(i).iterkeys():
			if j.endswith('_MAX'):
				del i[j]

	typedefs = extract_typedefs(ast)
	#export this in one dict so that we have a way to add it to parent scripts.
	all_info = {
	'functions' : extract_functions(ast = ast, typedefs = typedefs),
	'typedefs': typedefs,
	'constants' : constants,
	'constants_by_enum': constants_by_enum
	}

	metadata = metadata_handler.make_metadata()
	all_info['metadata'] = metadata

	#We can extract the "important" enums by looking for all properties with a value_enum key and grabbing its value.
	important_enums = []
	for i in metadata['nodes'].values():
		for j in i.get('properties', dict()).values():
			if 'value_enum' in j:
				important_enums.append(j['value_enum'])
	for i in metadata['additional_important_enums']:
		important_enums.append(i)

	all_info['important_enums'] = important_enums
	return all_info
