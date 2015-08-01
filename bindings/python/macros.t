{%macro implement_property(enumerant, prop)%}

	@property
	def {{prop['name']}}(self):   
		"""{{prop.get('doc_description', "")}}"""
{%if prop['type'] == 'int'%}
{%if 'value_enum' in prop%}
		return IntProperty(node = self, slot = _libaudioverse.{{enumerant}}, enum = {{prop['value_enum']|without_lav|underscores_to_camelcase(True)}})
{%else%}
		return IntProperty(node = self, slot = _libaudioverse.{{enumerant}})
{%endif%}
{%elif prop['type'] in ['int_array','float_array']%}
		return {{prop['type']|underscores_to_camelcase(True)}}Property(self, slot=_libaudioverse.{{enumerant}})
{%else%}
		return {{prop['type']|underscores_to_camelcase(True)}}Property(node=self, slot = _libaudioverse.{{enumerant}})
{%endif%}

{%if not prop['read_only']%}
	@{{prop['name']}}.setter
	def {{prop['name']}}(self, value):
		self.{{prop['name']}}.value=value
{%endif%}
{%endmacro%}

{%macro implement_event(name, index, info)%}
	@property
	def {{name}}_event(self):
		"""{{info.get('doc_description', "")}}"""
		evt = self._state['events'].get({{index}}, None)
		if evt is None:
			return
		return (evt.callback, evt.extra_arguments)

	@{{name}}_event.setter
	def {{name}}_event(self, val):
		global _global_events
		val_tuple = tuple(val) if isinstance(val, collections.Iterable) else (val, )
		if len(val_tuple) == 1:
			val_tuple = (val, ())
		cb, extra_args = val_tuple
		event_obj = _EventCallbackWrapper(self, {{index}}, cb, extra_args)
		self._state['events'][{{index}}] = event_obj
{%endmacro%}