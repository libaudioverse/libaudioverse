"""Implements standard transformers: Lav_ remover, camelcase-to-underscore and back, etc.

These are then made available to templates."""
import re

def without_lav(s):
	"""Remove the lav_ prefix from a Libaudioverse identifier."""
	if s.startswith('Lav_'):
		return s[4:]
	else:
		return s

def underscores_to_camelcase(s, capitalize_first = False):
	"""Convert an identifer of the form ab_cd_ef_gh to abCdEfGh.  If capitalize_first is True, convert to AbCdEfGh."""
	what = s.lower()
	what = re.sub('_([a-z])', lambda x: x.group(1).upper(), what)
	if capitalize_first:
		what = what[0].upper() + what[1:]
	return what

def camelcase_to_underscores(s):
	"""Converts camelcase identifiers to have underscores and be all lowercase."""
	what = s[0].lower()+s[1:]
	what =  re.sub('[A-Z]', lambda x: '_' + x.group(0).lower(), what)
	return what

def prefix_filter(l, prefix):
	"""Expects l, an iterable of strings, and a prefix.  Returns a list consisting of all strings in l that begin with prefix."""
	return filter(lambda x: x.startswith(prefix), l)

def remove_filter(l, item):
	"""Returns l without all instances of item."""
	return filter(lambda x: x != item, l)

def strip_prefix(s, prefix):
	assert s.startswith(prefix)
	return s[len(prefix):]

def get_jinja2_filters():
	return {'without_lav': without_lav,
		'camelcase_to_underscores': camelcase_to_underscores,
		'underscores_to_camelcase': underscores_to_camelcase,
		'remove_filter': remove_filter,
		'prefix_filter': prefix_filter,
		'strip_prefix': strip_prefix,
	}
