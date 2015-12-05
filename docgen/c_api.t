{%import 'macros.t' as macros  with context%}
[[c-api]]
== The Libaudioverse C API

This section of the Libaudioverse documentation documents the low-level C API.

[[c-api-functions]]
=== Functions By Category

The following is a complete listing of non-node-specific Libaudioverse functions.
If you do not see a function here, it is documented with its node.

Each function lists a description, the full C prototype, and information on all involved typedefs.
The `LavError` and `LavHandle` typedefs are intensionally omitted: both typedef to int.

All functions return error codes which should be checked.
Error  conditions are not documented at this time; see the enum section of this documentation for specific information on possible error return values.
Most error conditions are indicative of programmer error.

{%for category_info in metadata['function_categories']%}
==== {{category_info['doc_name']}}

{{category_info['doc_description']}}

{%for name in functions_by_category[category_info['name']]%}
{{macros.render_function(name, metadata['functions'][name])}}
{%endfor%}

{%endfor%}