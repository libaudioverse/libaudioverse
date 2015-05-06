from docgen.function_docs import *
import bindings.get_info as gi
x=gi.get_all_info()

for n, t in x['typedefs'].iteritems():
	print n, "=", type_to_string(t)

for n, f in x['functions'].iteritems():
	print "prototype for", n, "=", function_to_string(f)
	