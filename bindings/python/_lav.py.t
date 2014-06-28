{%-import 'macros.t' as macros-%}
#this template implements lifting the raw ctypes-basedd api into something markedly pallatable through judicious use of string manipulation and abuse, though a majority of that manipulation doesn't happen here.
#among other things, the implementation heree enables calling functions with keyword arguments and throws exceptions on error, rather than dealing with ctypes directly.

{%for func_name, func_info in high_level_functions%}
{%endfor%}
