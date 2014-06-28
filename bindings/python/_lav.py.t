{%-import 'macros.t' as macros-%}
#implements lifting the raw ctypes-basedd api into something markedly pallatable.
#among other things, the implementation heree enables calling functions with keyword arguments and throws exceptions on error, rather than dealing with ctypes directly.

import _libaudioverse

{%for func_name, friendly_name in friendly_functions.iteritems()%}
{%-set func_info = functions[func_name]-%}
{%-set input_arg_names = func_info.input_args|map(attribute='name')|list-%}
{%-if func_info.output_args|length == 0-%}
def {{friendly_name}}({{input_arg_names|join(', ')}}):
	err = {{func_name}}({{input_arg_names|join(', ')}})
	if err != _libaudioverse.Lav_ERROR_NONE:
		throw make_error_from_code(err)
{%endif%}
{%endfor%}
