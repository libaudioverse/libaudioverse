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

#initialize libaudioverse.  This is per-app and implies no context settings, etc.
_lav.initialize_library()
