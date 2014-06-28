{%-import 'macros.t' as macros-%}
#implements lifting the raw ctypes-basedd api into something markedly pallatable.
#among other things, the implementation heree enables calling functions with keyword arguments and throws exceptions on error, rather than dealing with ctypes directly.

import _libaudioverse

{%for func_name, friendly_name in friendly_functions.iteritems()%}
{%-set func_info = functions[func_name]-%}
def {{friendly_name}}({{func_info.args|map(attribute='name')|join(', ')}}):
	pass
{%endfor%}
