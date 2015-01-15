#demonstrates the HRTF panner.
import libaudioverse
import os.path
import time

libaudioverse.initialize()
sim = libaudioverse.Simulation(device_index = -1)
print "Enter a file path.  For best results, this should be a"
print "mono file; if not, only the first (usually left) channel will be used."
path = raw_input()
path = os.path.abspath(path)
fnode = libaudioverse.FileNode(sim, path)
fnode.looping = True
panner = libaudioverse.HrtfNode(sim, "default")
panner.inputs[0] = fnode, 0
sim.output_node=panner
print """Beginning evaluation.
Enter any python expression that returns a tuple of numbers.  The first is azimuth and the second is elevation.  Azimuth may be anything, but elevation must be on the range -90 to 90.
Enter quit to quit.
"""
while True:
	command = raw_input()
	if command == 'quit':
		break
	az, elev = eval(command)
	panner.azimuth = az
	panner.elevation = elev

libaudioverse.shutdown()