from .. import get_info
import jinja2
ctypes_map = {
'int' : 'c_int',
'unsigned int' : 'c_uint',
'float' : 'c_float',
'double' : 'c_double',
}

def make_python():
	context = dict()
	context.update(get_info.all_info)
	context['ctypes_map'] = ctypes_map
	env = jinja2.Environment(loader = jinja2.PackageLoader(__package__, ""), undefined = jinja2.StrictUndefined)
	template = env.get_template('ctypes.py.t')
	ct = template.render(context)
	return {'ctypes.py' : ct}
