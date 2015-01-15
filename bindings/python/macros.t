{%macro implement_property(enumerant, prop)%}
	@property
	def {{prop['name']}}(self):
{%if prop['type'] == 'int' and 'value_enum' in prop%}
		val = _lav.node_get_int_property(self.handle, _libaudioverse.{{enumerant}})
		return {{prop['value_enum']|without_lav|underscores_to_camelcase(True)}}(val)
{%elif prop['type'] == 'boolean'%}
		return bool(_lav.node_get_int_property(self.handle, _libaudioverse.{{enumerant}}))
{%elif 'array' not in prop['type']%}
		return _lav.node_get_{{prop['type']}}_property(self.handle, _libaudioverse.{{enumerant}})
{%elif prop['type'] == 'float_array'%}
		retval = []
		for i in xrange(_lav.node_get_float_array_property_length(self.handle, _libaudioverse.{{enumerant}})):
			retval.append(_lav.node_read_float_array_property(self.handle, _libaudioverse.{{enumerant}}, i))
		return tuple(retval)
{%elif prop['type'] == 'int_array'%}
		retval = []
		for i in xrange(_lav.node_get_int_array_property_length(self.handle, _libaudioverse.{{enumerant}})):
			retval.append(_lav.node_read_int_array_property(self.handle, _libaudioverse.{{enumerant}}, i))
		return tuple(retval)
{%endif%}

{%if prop.get('read_only', False) == False%}
	@{{prop['name']}}.setter
	def {{prop['name']}}(self, val):
{%if 'value_enum' in prop%}
		if not isinstance(val, {{prop['value_enum']|without_lav|underscores_to_camelcase(True)}}) and isinstance(val, enum.IntEnum):
			raise valueError('Attemptn to use wrong enum to set property. Expected instance of {{prop['value_enum']|without_lav|underscores_to_camelcase(True)}}')
		if isinstance(val, enum.IntEnum):
			val = val.value
{%endif%}
{%if prop['type'] == 'int'%}
		_lav.node_set_int_property(self.handle, _libaudioverse.{{enumerant}}, int(val))
{%elif prop['type'] == 'boolean'%}
		_lav.node_set_int_property(self.handle, _libaudioverse.{{enumerant}}, int(bool(val)))
{%elif prop['type'] == 'float' or prop['type'] == 'double'%}
		_lav.node_set_{{prop['type']}}_property(self.handle, _libaudioverse.{{enumerant}}, float(val))
{%elif prop['type'] == 'float3'%}
		arg_tuple = tuple(val)
		if len(arg_tuple) != 3:
			raise  ValueError('Expected a list or list-like object of 3 floats')
		_lav.node_set_float3_property(self.handle, _libaudioverse.{{enumerant}}, *(float(i) for i in arg_tuple))
{%elif prop['type'] == 'float6'%}
		arg_tuple = tuple(val)
		if len(arg_tuple) != 6:
			raise ValueError('Expected a list or list-like object of 6 floats')
		_lav.node_set_float6_property(self.handle, _libaudioverse.{{enumerant}}, *(float(i) for i in arg_tuple))
{%elif prop['type'] == 'float_array'%}
		if not isinstance(val, collections.Sized):
			raise ValueError('expected an iterable with known size')
		_lav.node_replace_float_array_property(self.handle, _libaudioverse.{{enumerant}}, len(val), val)
{%elif prop['type'] == 'int_array'%}
		if not isinstance(val, collections.Sized):
			raise ValueError('expected an iterable with known size')
		_lav.node_replace.Int_array_property(self.handle, _libaudioverse.{{enumerant}}, len(val), val)
{%endif%}
{%endif%}
{%endmacro%}

{%macro implement_event(name, index)%}
	@property
	def {{name}}_event(self):
		evt = self._events.get({{index}}, None)
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
		self._events[{{index}}] = event_obj
		_global_events[self.handle].add(event_obj)
{%endmacro%}