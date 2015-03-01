== Libaudioverse Object Reference

This page is an overview of all Libaudioverse objects.
The following tables completely describe all implemented Libaudioverse objects in terms of the C API.

For binding-specific references, see the bindings themselves.
Python uses docstrings, for example.
Also see the Binding-specific Notes section of this documentation.

Usually, the names of objects can be inferred by mentally stripping the `Lav_OBJTYPE_` prefix from the identifying enumerant and looking for `suffix_object` or `SuffixObject` depending on the conventions of your language.
These transformations are done for you in this document, listed as Camelcase Identifier and underscore identifier, respectively.

For a discussion of property types, see Property Types.  For a discussion of callbacksk see Callbacks.  This document is focused only on the objects themselves.

{%for node_name in sorted_nodes%}
{%set doc_header = nodes[node_name]['doc_name']%}
=== {{doc_header}}

{%if nodes[node_name].get('properties', [])|length > 0%}
==== Properties

|===
|Name|C Constant|Type|Range|Default | Description
{%for propinfo in nodes[node_name]['properties'].iteritems()%}

|{{propinfo[1]['name']}}
|{{propinfo[0]}}
|{{propinfo[1]['type']}}
|{{propinfo[1].get('range', '')}}
|{{propinfo[1].get('default', 'See Description')}}
|{{propinfo[1].get('doc_description', 'None Defined')}}
{%endfor%}
|===

{%endif%}
{%if nodes[node_name].get('events', [])|length > 0%}
==== Events

|===
|Name | C Constant | description
{%for callinfo in nodes[node_name]['events'].iteritems()%}

|{{callinfo[1]['name']}}
|{{callinfo[0]}}
|{{callinfo[1].get('doc_description', 'None Defined')}}
{%endfor%}
|===
{%endif%}

{%endfor%}