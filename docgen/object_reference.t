== Libaudioverse Object Reference

This page is an overview of all Libaudioverse objects.
The following tables completely describe all implemented Libaudioverse objects in terms of the C API.

For binding-specific references, see the bindings themselves.
Python uses docstrings, for example.
Also see the Binding-specific Notes section of this documentation.

Usually, the names of objects can be inferred by mentally stripping the `Lav_OBJTYPE_` prefix from the identifying enumerant and looking for `suffix_object` or `SuffixObject` depending on the conventions of your language.
These transformations are done for you in this document, listed as Camelcase Identifier and underscore identifier, respectively.

For a discussion of property types, see Property Types.  For a discussion of callbacksk see Callbacks.  This document is focused only on the objects themselves.

{%for obj_name in sorted_objects%}
{%set doc_header = metadata[obj_name]['doc_name']%}
=== {{doc_header}}

{%if metadata[obj_name].get('properties', [])|length > 0%}
==== Properties

|===
|Name|C Constant|Type|Range|Default | Description
{%for propinfo in metadata[obj_name]['properties'].iteritems()%}

|{{propinfo[1]['name']}}
|{{propinfo[0]}}
|{{propinfo[1]['type']}}
|{{propinfo[1].get('range', '')}}
|{{propinfo[1].get('default', 'See Description')}}
|{{propinfo[1].get('doc_description', 'None Defined')}}
{%endfor%}
|===

{%endif%}
{%if metadata[obj_name].get('callbacks', [])|length > 0%}
==== Callbacks

|===
|Name | C Constant | description
{%for callinfo in metadata[obj_name]['callbacks'].iteritems()%}

|{{callinfo[1]['name']}}
|{{callinfo[0]}}
|{{callinfo[1].get('doc_description', 'None Defined')}}
{%endfor%}
|===
{%endif%}

{%endfor%}