{%macro make_property_instance(enumerant, prop)%}
{%if prop['type'] == 'int'%}
{%if 'value_enum' in prop%}
self._state['property_instances'][_libaudioverse.{{enumerant}}] = IntProperty(handle = self.handle, slot = _libaudioverse.{{enumerant}}, enum = {{prop['value_enum']|without_lav|underscores_to_camelcase(True)}})
{%else%}
self._state['property_instances'][_libaudioverse.{{enumerant}}] = IntProperty(handle = self.handle, slot = _libaudioverse.{{enumerant}})
{%endif%}
{%elif prop['type'] in ['int_array','float_array']%}
self._state['property_instances'][_libaudioverse.{{enumerant}}] = {{prop['type']|underscores_to_camelcase(True)}}Property(handle = self.handle, slot=_libaudioverse.{{enumerant}}, lock = self._lock)
{%else%}
self._state['property_instances'][_libaudioverse.{{enumerant}}] = {{prop['type']|underscores_to_camelcase(True)}}Property(handle = self.handle, slot = _libaudioverse.{{enumerant}})
{%endif%}
{%endmacro%}

{%macro implement_property(enumerant, prop)%}

    @property
    def {{prop['name']}}(self):   
        """{{prop.get('doc_description', "")}}"""
        return self._state['property_instances'][_libaudioverse.{{enumerant}}]

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