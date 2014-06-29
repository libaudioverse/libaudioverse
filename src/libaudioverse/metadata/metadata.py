import yaml
import os.path
import os
import jinja2
import sys
if len(sys.argv) != 2:
	print"Invalid usage: do not have destination"
	sys.exit(1)

print "Generating", sys.argv[1]

#generates metadata.cpp from metadata.y and metadata.t, which must be in the same directory as this script.

directory = os.path.split(os.path.abspath(__file__))[0]
with file(os.path.join(directory, 'metadata.y')) as f:
	metadata = yaml.load(f)
#we change {{ and }} to <% and %> to avoid ambiguity when building arrays.
environment = jinja2.Environment(
variable_start_string = '<%', variable_end_string = '%>',
loader = jinja2.FileSystemLoader(directory),
undefined = jinja2.StrictUndefined,
extensions = ['jinja2.ext.loopcontrols'])

#we have to do some sanitizing for the template.
#the map we have here is actually very verbose, and can be flattened into something easily iterable.
#we can then take advantage of either std::pair or std::tuple as the keys.
property_map = dict()
for objkey, properties in metadata['properties'].iteritems():
	for propkey, propinfo in properties.iteritems():
		property_map[(objkey, propkey)] = propinfo

#the template will convert the types into enums via judicious use of if statements-we use it like augmented c, and prefer to do refactoring only there when possible.
#each property will be crammed into a property descriptor, but some of the ranges here are currently potentially unfriendly, most notably float3 and float6.
#we are going to convert all numbers into strings, and make them valid c identifiers.  Skip anything that is already a string.
def string_from_number(val, type):
	if type in {'float', 'float3', 'float6'}:
		return str(float(val))+'f'
	elif type == 'double':
		return str(float(val))
	elif type == 'int':
		return str(int(val))	
	else:
		print "Warning: Unrecognized type or value with type", type, "and value", val
		print "Returning val unchanged."
		return val

for propkey, propinfo in property_map.iteritems():
	for i, j in enumerate(list(propinfo.get('range', []))): #if we don't have a range, this will do nothing.
		if isinstance(j, basestring):
			continue #it's either MIN_INT, MAX_INT, INFINITY, -INFINITY, or another special identifier.  Pass through unchanged.
		#we're not worried about float3 or float6 logic, because those aren't allowed to have traditional ranges at the omoment-so this is it:
		propinfo['range'][i] = string_from_number(j, propinfo['type'])
	#Default handling logic.  If we don't have one and are int, float, or double we make it 0.
	if propinfo['type'] in {'int', 'float', 'double'}:
		propinfo['default'] = string_from_number(propinfo.get('default', 0), propinfo['type'])
	#otherwise, if we're float3 or float6, we do a loop for the same behavior.
	elif propinfo['type'] in {'float3', 'float6'}:
		for i, j in enumerate(list(propinfo.get('default', [0, 0, 0] if propinfo['type'] == 'float3' else [0, 0, 0, 0, 0, 0]))):
			propinfo['default'][i] = string_from_number(j, propinfo['type'])


#do the render, and write to the file specified on the command line.
context = {
'properties' : property_map
}

template = environment.get_template('metadata.t')
result = template.render(context)

if not os.path.exists(os.path.split(os.path.abspath(sys.argv[1]))[0]):
	os.makedirs(os.path.split(os.path.abspath(sys.argv[1]))[0])

with file(sys.argv[1], 'w') as f:
	f.write(result)
