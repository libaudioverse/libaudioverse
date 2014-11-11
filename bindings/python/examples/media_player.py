#a simple command line media player.
import libaudioverse
import os.path
libaudioverse.initialize()

sim = libaudioverse.Simulation()
print """Command line Media player.
Please enter the path to a file in a format supported by Libsndfile: typically wave or ogg."""
filepath = raw_input()
filepath = os.path.abspath(filepath)
filenode = libaudioverse.FileObject(sim, filepath)

#callback for when the file finishes.
def finished(obj):
	print "Finished playing."

filenode.end_event = finished

sim.output_object = filenode

commands = """Commands:
play
pause
pitch_bend <number>
seek <seconds>
quit
"""

print commands

while True:
	try:
		command = raw_input().split(" ")
		if command[0] == 'quit':
			break
		elif command[0] == 'play':
			filenode.suspended = False
		elif command[0] == 'pause':
			filenode.suspended = True
		elif command[0] == 'seek':
			to = float(command[1])
			filenode.position = to
		elif command[0] == 'pitch_bend':
			to = float(command[1])
			filenode.pitch_bend = to
	except Exception as e:
		print "Libaudioverse error.  Unrecognized command, or invalid syntax."
		print commands

libaudioverse.shutdown()