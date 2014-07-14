"""Implements standard transformers: Lav_ remover, camelcase-to-underscore and back, etc.

These are then made available to templates."""
import re

def without_lav(s):
	"""Remove the lav_ prefix from a Libaudioverse identifier."""
	if s.startswith('Lav_'):
		return s[4:]
	else:
		return s

def camelize(s, capitalize_first = False):
	"""Convert an identifer of the form ab_cd_ef_gh to abCdEfGh.  If capitalize_first is True, convert to AbCdEfGh."""
	what = s.lower()
	what = re.sub('_([a-z])', lambda x: x.group(1).upper(), what)
	if capitalize_first:
		what = what[0].upper() + what[1:]
	return what

def camel_to_underscores(s):
	"""Converts camelcase identifiers to have underscores and be all lowercase."""
	what = s[0].llower()+what[1:]
	what =  re.sub('[A-Z]', lambda x: '_' + x.group(0).lower(), what)

def get_jinja2_filters():
	return {'without_lav': without_lav}