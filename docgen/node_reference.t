== Libaudioverse  Node Reference

This page is an overview of all Libaudioverse nodes.

For binding-specific references, see the bindings themselves.
Python uses docstrings, for example.
Also see the Binding-specific Notes section of this documentation.

Usually, the names of  nodes can be inferred by mentally stripping the `Lav_OBJTYPE_` prefix from the identifying constant and looking for `suffix_object` or `SuffixObject` depending on the conventions of your language.
These transformations are done for you in this document, listed as Camelcase Identifier and underscore identifier, respectively.

For a discussion of property types, see Property Types.  For a discussion of callbacksk see Callbacks.  This document is focused only on the  nodes themselves.

Any function not described in the <<c-api,C API section>> is a "extra function", a function which breaks the usual property model.
Extra functions do any number of things and are documented with the node that they manipulate.

To determine the number of inputs and outputs a node has, as well as their channel counts, check the general description of the node and the description of its constructor in that order.
No node's output or input count can change after the node is created.
A few nodes make the input and output channel counts change depending on properties.
The most notable node of this type is the amplitude panner.

{%for node_name in sorted_nodes%}
{%set doc_header = (nodes[node_name]['doc_name']+" node") | title%}
=== {{doc_header}}

{{nodes[node_name]['doc_description']}}

{%if nodes[node_name]['has_properties']%}
==== Properties


{%for propinfo in nodes[node_name]['properties'].iteritems()%}

===== {{propinfo[1]['name']}}

C Enumeration Value: {{propinfo[0]}}

Type: {{propinfo[1]['type']}}

Range: {{propinfo[1].get('range', '')}}

Default Value: {{propinfo[1].get('default', 'See Description')}}

{{propinfo[1]['doc_description']}}

{%endfor%}


{%endif%}
{%if nodes[node_name]['has_events']%}
==== Events

{%for callinfo in nodes[node_name]['events'].iteritems()%}

===== {{callinfo[1]['name']}}
C Enumeration Value: {{callinfo[0]}}

{{callinfo[1]['doc_description']}}

{%endfor%}

{%endif%}

{%endfor%}