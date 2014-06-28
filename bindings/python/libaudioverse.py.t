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

#All of these get expanded by __init__ in the generic object to include the necessary properties.
{%-for object_name, friendly_name in friendly_objects.iteritems()%}
class {{friendly_name}}(GenericObject):
	pas
{%endfor%}

#initialize libaudioverse.  This is per-app and implies no context settings, etc.
_lav.initialize_library()
