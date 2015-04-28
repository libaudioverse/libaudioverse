{%macro implement_property(enumerant, prop)%}

	@property
	def {{prop['name']}}(self):
{%if prop['type'] == 'int'%}
{%if 'value_enum' in prop%}
			return IntProperty(node = self, slot = _libaudioverse.{{enumerant}}, enum = {{prop['value_enum']|without_lav|underscores_to_camelcase(True)}})
{%else%}
			return IntProperty(node = self, slot = _libaudioverse.{{enumerant}})
{%endif%}
{%else%}
			return {{prop['type']|underscores_to_camelcase(True)}}Property(node=self, slot = _libaudioverse.{{enumerant}})
{%endif%}
{%endmacro%}

{%macro implement_event(name, index)%}
	@property
	def {{name}}_event(self):
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
		_global_events[self.handle.handle].add(event_obj)
{%endmacro%}