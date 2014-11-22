#demonstrates how to use the 3d simulation.
import libaudioverse
import collections
libaudioverse.initialize()

sim = libaudioverse.Simulation(device_index = -1)
world = libaudioverse.World(sim, "mit.hrtf")
source = libaudioverse.Source(sim, world)
print """Enter a path to a sound file.
For best results, this should be mono.  If not, only the first (usually left) channel will be used."""
filepath = raw_input()
f = libaudioverse.File(sim, filepath)
f.looping = True
source.inputs[0] = f, 0
sim.output_object = world
print """Enter python expressions that evaluate to 3-tuples (x, y, z).
Positive x is to your right, positive y is above you, and positive z is behind you.
Enter quit to quit."""
while True:
	command = raw_input()
	if command == 'quit':
		break
	vect = eval(command)
	if not isinstance(vect, collections.Sized) or len(vect) != 3:
		print "Must evaluate to a 3-tuple.  Try again"
		continue
	source.position = vect

libaudioverse.shutdown()