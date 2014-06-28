import jinja2
from collections import OrderedDict
import re

ctypes_map = {
'int' : 'c_int',
'unsigned int' : 'c_uint',
'float' : 'c_float',
'double' : 'c_double',
}

def make_python(info):
	context = dict()
	context.update(info)
	context['ctypes_map'] = ctypes_map
	#compute friendly function names, and put them in a dict.
	friendly_functions = OrderedDict()
	for name, function_info in info['functions'].iteritems():
		without_lav = name[4:] #strip the Lav_.
		friendly_name =  re.sub('[A-Z]', lambda x: '_' + x.group(0).lower(), without_lav)
		friendly_functions[friendly_name] = function_info
	context['friendly_functions'] = friendly_functions
	env = jinja2.Environment(loader = jinja2.PackageLoader(__package__, ""), undefined = jinja2.StrictUndefined)
	return {
		'_libaudioverse.py' : env.get_template('_libaudioverse.py.t').render(context),
		'_lav.py' : env.get_template('_lav.py.t').render(context)
	}
