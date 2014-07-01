{%-import 'macros.t' as macros with context-%}
import _lav
import _libaudioverse

{%macro implement_property(enumerant, prop)%}
	@property
	def {{prop['name']}}(self):
		return _lav.object_get_{{prop['type']}}_property(self.handle, _libaudioverse.{{enumerant}})

	@{{prop['name']}}.setter
	def {{prop['name']}}(self, val):
{%if prop['type'] == 'int'%}
		_lav.object_set_int_property(self.handle, _libaudioverse.{{enumerant}}, int(val))
{%elif prop['type'] == 'float' or prop['type'] == 'double'%}
		_lav.object_set_{{prop['type']}}_property(self.handle, _libaudioverse.{{enumerant}}, float(val))
{%elif prop['type'] == 'float3'%}
		arg_tuple = tuple(val)
		if len(arg_tuple) != 3:
			raise  ValueError('Expected a list or list-like object of 3 floats')
		_lav.object_set_float3_property(self.handle, _libaudioverse.{{enumerant}}, *(float(i) for i in arg_tuple))
{%elif prop['type'] == 'float6'%}
		arg_tuple = tuple(val)
		if len(arg_tuple) != 6:
			raise ValueError('Expected a list or list-like object of 6 floats')
		_lav.object_set_float6_property(self.handle, _libaudioverse.{{enumerant}}, *(float(i) for i in arg_tuple))
{%endif%}
{%endmacro%}

#This dict maps type codes to classes so that we can revive objects from handles, etc.
_types_to_classes = dict()

#build and register all the error classes.
class GenericError(object):
	"""Base for all libaudioverse errors."""
	pass
{%for error_name, friendly_name in friendly_errors.iteritems()%}
class {{friendly_name}}(GenericError):
	pass
_lav.bindings_register_exception(_libaudioverse.{{error_name}}, {{friendly_name}})
{%endfor%}

#This is the class hierarchy.
#GenericObject is at the bottom, and we should never see one; and GenericObject should hold most implementation.
class GenericObject(object):
	"""A Libaudioverse object."""

	def __init__(self, handle):
		self.handle = handle

{%for enumerant, prop in properties['Lav_OBJTYPE_GENERIC'].iteritems()%}
{{implement_property(enumerant, prop)}}
{%endfor%}

_types_to_classes[_libaudioverse.Lav_OBJTYPE_GENERIC] = GenericObject

{%-for object_name, friendly_name in friendly_objects.iteritems()%}
{%set constructor_arg_names = object_constructor_info[object_name].input_args|map(attribute='name')|list-%}
class {{friendly_name}}(GenericObject):
	def __init__(self{%if constructor_arg_names|length > 0%}, {%endif%}{{constructor_arg_names|join(', ')}}):
		super({{friendly_name}}, self).__init__(_lav.{{object_constructors[object_name]}}({{constructor_arg_names|join(', ')}}))
{%for enumerant, prop in properties.get(object_name, dict()).iteritems()%}
{{implement_property(enumerant, prop)}}

{%endfor%}
_types_to_classes[_libaudioverse.{{object_name}}] = {{friendly_name}}
{%endfor%}

#initialize libaudioverse.  This is per-app and implies no context settings, etc.
_lav.initialize_library()
