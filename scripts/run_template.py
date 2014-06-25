"""Extracts nodes of interest from a pycparser parse of bindings.h run through a platform-specific preprocessor, and then calls jinja2 on the passed argument."""

from pycparser import *
#we need to be able to compare with isinstance, unfortunately. Grab all of these too.
from pycparser.c_ast import *
import subprocess
import sys
import os.path

#this is a helper class representing a type.
#base is int, etc.
#indirection is the number of *s. int* is 1, etc.
class TypeInfo(object):
	def __init__(self, base, indirection):
		self.base = base
		self.indirection = indirection

#compute the input file.
#this gives us the root directory of the repository.
root_directory = os.path.split(os.path.split(os.path.abspath(__file__))[0])[0]
input_file = os.path.join(root_directory, 'include', 'libaudioverse', 'binding.h')

if sys.platform == 'win32':
	command = 'cl'
	args = '/EP ' + input_file
else:
	command  = 'cpp'
	args = input_file

text = subprocess.check_output(command + ' ' + args, shell = True)
#convert from windows to linux newlines, if needed.
text = text.replace('\r\n', '\n')

#build a cffi parser.
parser = c_parser.CParser()
ast = parser.parse(text)

#do the low-hanging fruit first: enums are simple enough.
#we don't allow declaring nested types, so we know that all enums are at the top level-that is, they are to be found in ast.ext.
enum_list = [i.type for i in ast.ext if isinstance(i, Decl) and isinstance(i.type, Enum)]

#now, we note that we can find--for a specific enum e--:
#name = e.name, values are in e.values as e.values[index].value and names in e.values[index]name
#ironically, we actually only care about the values of the enums.
#note that minimal interpretation is needed so that we can have negated constants-pycparser is for interpreters, not this, and so represents them as a unary minus in the ast.
constants = dict()

for enum in enum_list:
	for enum_value in enum.values.enumerators:
		val = enum_value.value
		if isinstance(val, Constant):
			constants[enum_value.name] = int(enum_value.value.value)
		elif isinstance(val, UnaryOp) and val.op == '-':
			constants[enum_value.name] = int('-' + val.expr.value)

#pull out typedefs.
#again, we expect them at the top levle--if they're not, we'll miss them.
#the primary use of this is a bit later, when we build function objects-we aggregate typedefs when possible.
#at the moment, we typedef everything to void. This is not likely to change, so the following block basically works.
typedef_list = [i for i in ast.ext if isinstance(i, Typedef)]
typedefs = dict()

for typedef in typedef_list:
	name = typedef.name
	base = typedef.type.type
	if isinstance(base, IdentifierType):
		base = base.names
	elif isinstance(base, Enum):
		base = base.name
	indirection = 0
	typedefs[name] = TypeInfo(base, indirection)
