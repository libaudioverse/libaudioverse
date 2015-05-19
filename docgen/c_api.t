{%import 'macros.t' as macros  with context%}
[[c-api]]
== The Libaudioverse C API

This section of the Libaudioverse documentation documents the low-level C API.

[[c-api-functions]]
=== Functions By Category

The following is a complete listing of non-node-specific Libaudioverse functions.
if you do not see a function here, it is documented with its node.

Each function lists a description, the full C prototype, and information on all involved typedefs.
This manual does not have a dedicated section for typedefs as there are a sizeable number of them, and most are used with only one or two functions.
The `LavError` and `LavHandle` typedefs are intensionally omitted: both typedef to int.

All functions return error codes which should be checked.
Error  conditions are not documented; see the enum section of this documentation for specific information on possible error return values.

{%for category_info in metadata['function_categories']%}
==== {{category_info['doc_name']}}

{{category_info['doc_description']}}

{%for name in functions_by_category[category_info['name']]%}
{{macros.render_function(name, metadata['functions'][name])}}
{%endfor%}

{%endfor%}