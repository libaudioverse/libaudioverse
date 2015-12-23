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
        """Type: {{prop['type']}}

{%if not prop['read_only']%}
{%-if 'value_enum' in prop%}Range: :any:`{{prop['value_enum']|without_lav|underscores_to_camelcase(True)}}`
{%-elif 'range' in prop and prop['range']|length == 2%}Range: [{{prop['range'][0]}}, {{prop['range'][1]}}]
{%-elif 'range' in prop and prop['type'] != 'boolean'%}Range: {{prop['range']}}
{%-endif%}

{%if 'default' in prop and 'value_enum' in prop-%}
{%-set classname = prop['value_enum']|without_lav|underscores_to_camelcase(True)-%}
{%-set membername = prop['default']|strip_prefix(common_prefix(constants_by_enum[prop['value_enum']].keys()))|lower-%}
Default value: :any:`{{classname}}.{{membername}}`
{%-elif 'default' in prop and prop['type'] == 'boolean'%}Default value: {%if prop['default']%}True{%else%}False{%endif%}
{%-elif 'default' in prop%}Default value: {{prop['default']}}
{%-endif%}
{%-else%}This property is read-only.
{%-endif%}

{{prop.get('doc_description', "")}}"""
        return self._property_instances[_libaudioverse.{{enumerant}}]

{%if not prop['read_only']%}
    @{{prop['name']}}.setter
    def {{prop['name']}}(self, value):
        self.{{prop['name']}}.value=value
{%endif%}
{%endmacro%}
