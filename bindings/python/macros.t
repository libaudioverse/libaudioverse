{%macro make_property_instance(enumerant, prop)%}
{%if prop['type'] == 'int'%}
{%if 'value_enum' in prop%}
self._property_instances[_libaudioverse.{{enumerant}}] = IntProperty(handle = self.handle, slot = _libaudioverse.{{enumerant}}, enum = {{prop['value_enum']|without_lav|underscores_to_camelcase(True)}})
{%else%}
self._property_instances[_libaudioverse.{{enumerant}}] = IntProperty(handle = self.handle, slot = _libaudioverse.{{enumerant}})
{%endif%}
{%elif prop['type'] in ['int_array','float_array']%}
self._property_instances[_libaudioverse.{{enumerant}}] = {{prop['type']|underscores_to_camelcase(True)}}Property(handle = self.handle, slot=_libaudioverse.{{enumerant}}, lock = self._lock)
{%else%}
self._property_instances[_libaudioverse.{{enumerant}}] = {{prop['type']|underscores_to_camelcase(True)}}Property(handle = self.handle, slot = _libaudioverse.{{enumerant}})
{%endif%}
{%endmacro%}

{%macro implement_property(enumerant, prop)%}

    @property
    def {{prop['name']}}(self):   
        """{{prop.get('doc_description', "")}}"""
        return self._property_instances[_libaudioverse.{{enumerant}}]

{%if not prop['read_only']%}
    @{{prop['name']}}.setter
    def {{prop['name']}}(self, value):
        self.{{prop['name']}}.value=value
{%endif%}
{%endmacro%}
