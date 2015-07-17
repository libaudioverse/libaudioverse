#demonstrates the HRTF panner.
import libaudioverse
import os.path
import time

libaudioverse.initialize()
sim = libaudioverse.Simulation()
sim.set_output_device(-1)
print "Enter a file path."
path = raw_input()
path = os.path.abspath(path)
fnode = libaudioverse.BufferNode(sim)
buffer=libaudioverse.Buffer(sim)
buffer.load_from_file(path)
fnode.buffer = buffer
fnode.looping.value = True
panner = libaudioverse.HrtfNode(sim, "default")
fnode.connect(0, panner, 0)
panner.connect_simulation(0)

print """Beginning evaluation.
Enter any python expression that returns a tuple of numbers.  The first is azimuth and the second is elevation.  Azimuth may be anything, but elevation must be on the range -90 to 90.
Enter quit to quit.
"""
while True:
	command = raw_input()
	if command == 'quit':
		break
	az, elev = eval(command)
	panner.azimuth.value = az
	panner.elevation.value = elev

libaudioverse.shutdown()