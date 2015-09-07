{%-import 'macros.t' as macros with context-%}
#implements lifting the raw ctypes-basedd api into something markedly pallatable.
#among other things, the implementation heree enables calling functions with keyword arguments and raises exceptions on error, rather than dealing with ctypes directly.
import ctypes
import collections
import functools
import _libaudioverse

#These are not from libaudioverse.
#Implement a method by which the public libaudioverse module may register its exception classes for error code translation.
class PythonBindingsCouldNotTranslateErrorCodeError(Exception):
	"""An exception representing failure to translate a libaudioverse error code into a python exception.  If you see this, report it as a bug with Libaudioverse because something has gone very badly wrong."""
	pass

errors_to_exceptions = dict()

def bindings_register_exception(code, cls):
	errors_to_exceptions[code] = cls

def make_error_from_code(err):
	"""Internal use.  Translates libaudioverse error codes into exceptions."""
	return errors_to_exceptions.get(err, PythonBindingsCouldNotTranslateErrorCodeError)()

#Handle marshalling and automatic refcount stuff:
@functools.total_ordering
class _HandleBox(object):

	def __init__(self, handle):
		self.handle= int(handle)
		first_access= ctypes.c_int()
		_libaudioverse.Lav_handleGetAndClearFirstAccess(handle, ctypes.byref(first_access))
		if not first_access:
			_libaudioverse.Lav_handleIncRef(handle)

	def __eq__(self, other):
		if not isinstance(other, _HandleBox): return False
		else: return self.handle == other.handle

	def __lt__(self, other):
		if not isinstance(other, _HandleBox): return True #other classes are "less" than us.
		return self.handle < other.handle

	def __hash__(self):
		return self.handle

	def __del__(self):
		#Guard against interpreter shutdown.
		if self.handle is None or _libaudioverse is None: return
		_libaudioverse.Lav_handleDecRef(self.handle)
		self.handle = None

def reverse_handle(handle):
	return _HandleBox(handle)

{%macro autopointerize(arglist)%}
{%for arg in arglist%}
{%if arg.type.base == 'LavHandle'%}
	#Drill down up to twice, otherwise assume we passed in something safe.
	{{arg.name}} = getattr({{arg.name}}, 'handle', {{arg.name}})
	{{arg.name}} = getattr({{arg.name}}, 'handle', {{arg.name}})
{%endif%}
{%if arg.type.indirection == 1 and not arg.type.base == 'char'%}
	if isinstance({{arg.name}}, collections.Sized):
		if not isinstance({{arg.name}}, basestring):
			{{arg.name}}_t = {{arg.type|ctypes_string(1)}}*len({{arg.name}})
			#Try to use the buffer interfaces, if we can.
			try:
				{{arg.name}} = {{arg.name}}_t.from_buffer({{arg.name}})
			except TypeError:
				{{arg.name}}_new = {{arg.name}}_t()
				for i, j in enumerate({{arg.name}}):
					{{arg.name}}_new[i] = j
				{{arg.name}} = {{arg.name}}_new
		else:
			{{arg.name}} = ctypes.cast(ctypes.create_string_buffer({{arg.name}}, len({{arg.name}})), {{arg.type|ctypes_string}})
{%endif%}
{%endfor%}
{%endmacro%}

{%for func_name, func_info in functions.iteritems()%}
{%set friendly_name = func_name|without_lav|camelcase_to_underscores%}
{%set input_arg_names = func_info.input_args|map(attribute='name')|list%}
{%set output_arg_names = func_info.output_args|map(attribute='name')|list%}
{%if func_info.output_args|length == 0%}
def {{friendly_name}}({{input_arg_names|join(', ')}}):
{{autopointerize(func_info.input_args)}}
	err = _libaudioverse.{{func_name}}({{input_arg_names|join(', ')}})
	if err != _libaudioverse.Lav_ERROR_NONE:
		raise make_error_from_code(err)
{%else%}
def {{friendly_name}}({{input_arg_names|join(', ')}}):
{{autopointerize(func_info.input_args)}}
{%for i in func_info.output_args%}
	{{i.name}} = {{i.type|ctypes_string(1, '_libaudioverse.')}}()
{%endfor%}
	err = _libaudioverse.{{func_name}}({{input_arg_names|join(', ')}}{%if input_arg_names|length > 0%}, {%endif%}
		{%for i in output_arg_names%}ctypes.byref({{i}}){%if not loop.last%}, {%endif%}{%endfor%})
	if err != _libaudioverse.Lav_ERROR_NONE:
		raise make_error_from_code(err)
	retval = []
{%for i in func_info.output_args%}
{%if i.type.base=='LavHandle' and i.type.indirection == 1%}
	handle = {{i.name}}.value
	retval.append(reverse_handle(handle))
{%else%}
	retval.append(getattr({{i.name}}, 'value', {{i.name}}))
{%endif%}
{%endfor%}
	return tuple(retval) if len(retval) > 1 else retval[0]
{%endif%}

{%endfor%}
