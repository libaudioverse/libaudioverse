import jinja2
from collections import OrderedDict
import re
from .. import transformers
from .. import get_info

ctypes_map = {
'int' : 'c_int',
'unsigned int' : 'c_uint',
'float' : 'c_float',
'double' : 'c_double',
}

def ctypes_string(typeinfo, offset = 0):
	"""Convert a type to a ctypes string.  Offset is used by the template _lav.py.t to make output argument strings."""
	if offset != 0:
		assert typeinfo.indirection-offset >= 0
		return ctypes_string(get_info.TypeInfo(typeinfo.base, typeinfo.indirection-offset))
	if typeinfo.indirection == 1 and typeinfo.base == 'void':
		return "ctypes.c_void_p"
	elif typeinfo.indirection == 0:
		return "ctypes." + ctypes_map[typeinfo.base]
	else:
		return "ctypes.POINTER(" + ctypes_string(get_info.TypeInfo(typeinfo.base, typeinfo.indirection-1)) + ")"

def make_python(info):
	context = dict()
	context.update(info)
	context['ctypes_map'] = ctypes_map
	#compute friendly function names, and put them in a dict.
	friendly_functions = OrderedDict()
	for name, function_info in info['functions'].iteritems():
		without_lav = name[4:] #strip the Lav_.
		friendly_name =  re.sub('[A-Z]', lambda x: '_' + x.group(0).lower(), without_lav)
		friendly_functions[name] = friendly_name
	context['friendly_functions'] = friendly_functions
	#similar logic for error constants, only this time we're converting them to python class names.
	friendly_errors = OrderedDict()
	for i in [j for j in info['constants'].iterkeys() if j.startswith('Lav_ERROR_')]:
		if i == 'Lav_ERROR_NONE':
			continue #this doesn't map to an exception.
		friendly_name = i.lower()
		friendly_name = re.sub('_([a-z])', lambda x: x.group(1).upper(), friendly_name)
		friendly_name = friendly_name[len('lavError'):] #strip leading lavError, left over from the above conversion.
		friendly_name += 'Error' #tag the end of it for the class names.
		friendly_errors[i] = friendly_name
	context['friendly_errors']  = friendly_errors
	#we also do something for the objtype enum along the same lines.
	friendly_objects = OrderedDict()
	object_constructors = OrderedDict()
	object_constructor_info = OrderedDict()
	for i in [j for j in info['constants'].iterkeys() if j.startswith('Lav_OBJTYPE_')]:
		if i == 'Lav_OBJTYPE_GENERIC':
			continue #generic objects are the base of the hierarchy, and are implemented by BaseObject class.
		friendly_name = i.lower()
		friendly_name = re.sub('_([a-z])', lambda x: x.group(1).upper(), friendly_name)
		friendly_name = friendly_name[len('lavObjtype'):] + 'Object'
		friendly_objects[i] = friendly_name
		constructor_name = i.lower()
		constructor_name = constructor_name[len('lav_objtype_'):]
		constructor_name = 'create_' + constructor_name + '_object'
		object_constructors[i] = constructor_name
		info_name = 'Lav_create' + friendly_name
		object_constructor_info[i] = info['functions'][info_name]
	context['friendly_objects'] = friendly_objects
	context['object_constructors'] = object_constructors
	context['object_constructor_info'] = object_constructor_info
	env = jinja2.Environment(loader = jinja2.PackageLoader(__package__, ""), undefined = jinja2.StrictUndefined)
	env.filters.update(transformers.get_jinja2_filters())
	env.filters['ctypes_string'] = ctypes_string
	return {
		'_lav.py' : env.get_template('_lav.py.t').render(context),
		'_libaudioverse.py' : env.get_template('_libaudioverse.py.t').render(context),
		'libaudioverse.py': env.get_template('libaudioverse.py.t').render(context)
	}
