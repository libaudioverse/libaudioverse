from .. import get_info
import jinja2

def make_python():
	context = dict()
	context.update(get_info.all_info)
	env = jinja2.Environment(loader = jinja2.PackageLoader(__package__, ""))
	template = env.get_template('ctypes.py.t')
	print template.render(context)
