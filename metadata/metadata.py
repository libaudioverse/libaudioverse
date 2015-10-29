import yaml
import os.path
import os
import jinja2
import sys
sys.path = [os.path.join(os.path.dirname(__file__), '..')] + sys.path
import bindings.get_info
from bindings.maybe_write import maybe_write
import re

if len(sys.argv) != 2:
    print("Invalid usage: do not have destination")
    sys.exit(1)

print("Generating", sys.argv[1])

directory = os.path.split(os.path.abspath(__file__))[0]
all_info = bindings.get_info.get_all_info()
metadata =all_info['metadata']
#we change {{ and }} to <% and %> to avoid ambiguity when building arrays.
environment = jinja2.Environment(
variable_start_string = '<%', variable_end_string = '%>',
loader = jinja2.FileSystemLoader(directory),
undefined = jinja2.StrictUndefined,
extensions = ['jinja2.ext.loopcontrols'])

#we have to do some sanitizing for the template.
#the map we have here is actually very verbose, and can be flattened into something easily iterable.
#we can then take advantage of either std::pair or std::tuple as the keys.
joined_properties = []
for nodekey, nodeinfo in [(i, metadata['nodes'].get(i, dict())) for i in all_info['constants'].keys() if re.match("Lav_OBJTYPE(\w+_*)+_NODE", i)]:
    #add everything from the object itself.
    for propkey, propinfo in nodeinfo.get('properties', dict()).items():
        joined_properties.append((nodekey, propkey, propinfo))
    #if we're not suppressing inheritence, we follow this up with everything from lav_OBJTYPE_GENERIC.
    if not nodeinfo.get('suppress_implied_inherit', False):
        for propkey, propinfo in metadata['nodes']['Lav_OBJTYPE_GENERIC_NODE']['properties'].items():
            joined_properties.append((nodekey, propkey, propinfo))

#this is the same logic, but for events.
joined_events = []
for nodekey, nodeinfo in [(i, metadata['nodes'].get(i, dict())) for i in all_info['constants'].keys() if re.match("Lav_OBJTYPE_(\w+_*)+_NODE", i)]:
    #add everything from the object itself.
    for eventkey, eventinfo in nodeinfo.get('events', dict()).items():
        joined_events.append((nodekey, eventkey, eventinfo))
    #if we're not suppressing inheritence, we follow this up with everything from lav_OBJTYPE_GENERIC.
    if not nodeinfo.get('suppress_implied_inherit', False):
        for eventkey, eventinfo in metadata['nodes']['Lav_OBJTYPE_GENERIC_NODE'].get('events', dict()).items():
            joined_events.append((nodekey, eventkey, eventinfo))

#the template will convert the types into enums via judicious use of if statements-we use it like augmented c, and prefer to do refactoring only there when possible.
#each property will be crammed into a property descriptor, but some of the ranges here are currently potentially unfriendly, most notably float3 and float6.
#we are going to convert all numbers into strings, and make them valid c identifiers.  Skip anything that is already a string.
def string_from_number(val, type):
    if type in {'float', 'float3', 'float6', 'float_array'}:
        return str(float(val))+'f'
    elif type == 'double':
        return str(float(val))
    elif type in {'int', 'int_array', 'boolean'}:
        return str(int(val))    
    else:
        print("Warning: Unrecognized type or value with type", type, "and value", val)
        print("Returning val unchanged.")
        return val

#note that there is a very interesting little hack heere.
#Since the second key of every tuple is a dict, it's possible that we're marking up a property we've already marked up.
#Problem: if we just do dict(foo), and later nest dicts, we'll have hard-to-find problems.
#solution: mark the properties as we normalize them.
for propkey, propid, propinfo in joined_properties:
    if propinfo.get('normalized', False):
        continue
    if propinfo['type'] == 'boolean':
        propinfo['range'] = [0, 1]
    if propinfo['type'] == 'int' and propinfo.get('read_only', False):
        propinfo['range'] = [0, 0]
    if propinfo['type'] == 'float' and propinfo.get('read_only', False):
        propinfo['range'] = [0.0, 0.0]
    if propinfo['type'] == 'double' and propinfo.get('read_only', False):
        propinfo['range'] = [0.0, 0.0]
    #Some int arrays are arrays of enums, i.e. FDN filter types.
    if propinfo['type'] in {'int', 'int_array'} and not propinfo.get('read_only', False) and 'value_enum' in propinfo:
        e = all_info['constants_by_enum'][propinfo['value_enum']]
        r1 = min(e.values())
        r2 = max(e.values())
        propinfo['range'] = [r1, r2]
    if propinfo['type'] == 'int' and propinfo.get('default', None) in all_info['constants']:
        propinfo['default'] = all_info['constants'][propinfo['default']]
    if propinfo.get('range', None) =='dynamic':
        propinfo['range'] = [0, 0]
        propinfo['is_dynamic'] = True
    for i, j in enumerate(list(propinfo.get('range', []))): #if we don't have a range, this will do nothing.
        if isinstance(j, str):
            continue #it's either MIN_INT, MAX_INT, INFINITY, -INFINITY, or another special identifier.  Pass through unchanged.
        #we're not worried about float3 or float6 logic, because those aren't allowed to have traditional ranges at the moment-so this is it:
        propinfo['range'][i] = string_from_number(j, propinfo['type'])
    #Some array properties (see the feedback delay network) are controlled completely by their constructor.
    #If this is the case, we need to give it some defaults so that the generated cpp file doesn't explode.
    #The constructor later updates this information.
    if propinfo['type'] in {'int_array', 'float_array'} and propinfo.get('dynamic_array', False):
        propinfo['min_length'] = propinfo.get('min_length', 1)
        propinfo['max_length'] = propinfo.get('max_length', 1)
        #This string is handled later; we need to be careful about the type here.
        #No need to duplicate code we have to do in a minute anyway.
        propinfo['default'] = propinfo.get('default', 'zeros')
        propinfo['range'] = propinfo.get('range', ['MIN_INT', 'MAX_INT'] if propinfo['type'] == 'int_array' else ['-INFINITY', 'INFINITY'])
    #Default handling logic.  If we don't have one and are int, float, or double we make it 0.
    if propinfo['type'] in {'int', 'float', 'double'}:
        propinfo['default'] = string_from_number(propinfo.get('default', 0), propinfo['type'])
    #otherwise, if we're one of the array types, we do a loop for the same behavior.
    elif propinfo['type'] in {'float3', 'float6'}:
        for i, j in enumerate(list(propinfo.get('default', [0, 0, 0] if propinfo['type'] == 'float3' else [0, 0, 0, 0, 0, 0]))):
            propinfo['default'][i] = string_from_number(j, propinfo['type'])
    elif propinfo['type'] in {'int_array', 'float_array'}:
        #As a special case, we allow the default heere to be the string "zeros".
        if propinfo.get('default', None) == "zeros":
            length = int(propinfo['min_length'])
            propinfo['default'] = [0.0]*length
            if propinfo['type'] == 'float_array':
                propinfo['default'] = [int(i) for i in propinfo['default']]
        for i, j in enumerate(list(propinfo.get('default', []))):
            propinfo['default'][i] = string_from_number(j, propinfo['type'])
    propinfo['normalized'] = True

#Sorting these makes sure maybe-write always gets the same thing.
joined_properties.sort()
joined_events.sort()

#do the render, and write to the file specified on the command line.
context = {
'joined_properties': joined_properties,
'joined_events': joined_events,
}
context.update(all_info)

template = environment.get_template('metadata.t')
result = template.render(context)

if not os.path.exists(os.path.split(os.path.abspath(sys.argv[1]))[0]):
    os.makedirs(os.path.split(os.path.abspath(sys.argv[1]))[0])

maybe_write(sys.argv[1], result)
