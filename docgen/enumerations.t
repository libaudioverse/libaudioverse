{%import 'macros.t' as macros with context%}

== Enumerations

The following section lists some of Libaudioverse's enumerations.
This section concerns itself only with those enumerations that are important by themselves.

{%for name, info in metadata['enumerations'].items()%}

[[enum-{{name}}]]
=== {{name}}

{{info['doc_description']}}

[caption=""]
.Members
|===
| Member | Description
{%for member in constants_by_enum[name].keys()%}
| {{member}}
| {{info['members'].get(member, "Undocumented.")}}
{%endfor%}
|===

{%endfor%}
