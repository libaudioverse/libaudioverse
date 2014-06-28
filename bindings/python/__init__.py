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
	env = jinja2.Environment(loader = jinja2.PackageLoader(__package__, ""), undefined = jinja2.StrictUndefined)
	return {
		'_lav.py' : env.get_template('_lav.py.t').render(context),
		'_libaudioverse.py' : env.get_template('_libaudioverse.py.t').render(context),
		'libaudioverse.py': env.get_template('libaudioverse.py.t').render(context)
	}
