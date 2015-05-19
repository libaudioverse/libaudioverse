{%macro render_function(c_name, info_dict)%}
{%set name = info_dict.get('name', c_name)%}
{%set function_object = functions[c_name]%}
{%set function_documentation = info_dict['doc_description']%}
===== {{name}}

Prototype: `{{function_object | function_to_string}}`

{{function_documentation}}

{%if function_object.args|length%}
[caption=""]
.PARAMETERS
|====
|Type |Name |Description
{%for arg in function_object.args%}
|`{{arg.type | type_to_string}}`
|{{arg.name}}
|{{info_dict['params'][arg.name]}}
{%endfor%}
|====
{%endif%}

{%set involved_typedefs= function_object | compute_involved_typedefs%}
{%if involved_typedefs | length%}
[caption =""]
.Typedefs of interest
|====
|Name |Actual Type
{%for name, typedef in involved_typedefs%}
|{{name}}
|`{{typedef|type_to_string}}`
{%endfor%}
|====
{%endif%}

{%endmacro%}