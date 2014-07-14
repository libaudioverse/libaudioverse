"""Implements standard transformers: Lav_ remover, camelcase-to-underscore and back, etc.

These are then made available to templates."""

def without_lav(s):
	"""Remove the lav_ prefix from a Libaudioverse identifier."""
	if s.startswith('Lav_'):
		return s[4:]
	else:
		return s

def get_jinja2_filters():
	return {'without_lav': without_lav}