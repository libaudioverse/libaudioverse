{%macro render_args_table(header, function_object, arg_dict)%}
[caption=""]
.{{header}}
|===
|Type |Name |Description
{%for arg in function_object.args%}
|`pass:[{{arg.type | type_to_string}}]`
|{{arg.name}}
|{{arg_dict[arg.name]}}
{%endfor%}
|===

{%endmacro%}

{%macro render_typedefs_table(header, function_object)%}
{%set involved_typedefs= function_object | compute_involved_typedefs%}
[caption =""]
.{{header}}
|===
|Name |Actual Type
{%for name, typedef in involved_typedefs%}
|{{name}}
|`pass:[{{typedef|type_to_string}}]`
{%endfor%}
|===

{%endmacro%}

{%macro render_function(c_name, info_dict)%}
{%set name = info_dict.get('name', c_name)%}
{%set function_object = functions[c_name]%}
{%set function_documentation = info_dict['doc_description']%}

[[function-{{c_name}}]]
===== {{name}}

Prototype: `{{function_object | function_to_string}}`

{{function_documentation}}

{%if function_object.args|length%}
{{render_args_table("Parameters", function_object, info_dict['params'])}}
{%endif%}

{%if function_object|compute_involved_typedefs|length%}
{{render_typedefs_table("Typedefs", function_object)}}
{%endif%}

{%endmacro%}