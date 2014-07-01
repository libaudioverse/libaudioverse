{%-import 'macros.t' as macros with context-%}
import _lav
import _libaudioverse

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
	@property
	def {{prop['name']}}(self):
		pass

	@{{prop['name']}}.setter
	def {{prop['name']}}(self, val):
		pass
{%endfor%}

{%-for object_name, friendly_name in friendly_objects.iteritems()%}
{%set constructor_arg_names = object_constructor_info[object_name].input_args|map(attribute='name')|list-%}
class {{friendly_name}}(GenericObject):
	def __init__(self{%if constructor_arg_names|length > 0%}, {%endif%}{{constructor_arg_names|join(', ')}}):
		super({{friendly_name}}, self).__init__(_lav.{{object_constructors[object_name]}}({{constructor_arg_names|join(', ')}}))
{%for enumerant, prop in properties.get(object_name, dict()).iteritems()%}
	@property
	def {{prop['name']}}(self):
		pass

	@{{prop['name']}}.setter
	def {{prop['name']}}(self, val):
		pass
{%endfor%}
{%endfor%}

#initialize libaudioverse.  This is per-app and implies no context settings, etc.
_lav.initialize_library()
