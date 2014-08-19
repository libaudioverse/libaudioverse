#a simple command line media player.
import libaudioverse
import os.path

dev = libaudioverse.Device(physical_output_index = -1)
print """Command line Media player.
Please enter the path to a file in a format supported by Libsndfile: typically wave or ogg.
Mono or stereo files will work fine.  Surround sound files will have additional channels stripped, keeping only the first two."""
filepath = raw_input()
filepath = os.path.abspath(filepath)
filenode = libaudioverse.FileObject(dev, filepath)
mixer = libaudioverse.MixerObject(dev, max_parents = 1, inputs_per_parent = 2) #for upconverting mono files.
if filenode.output_count == 1:
	mixer.inputs[0] = filenode, 0
	mixer.inputs[1] = filenode, 0
else:
	mixer.inputs[0] = filenode, 0
	mixer.inputs[1] = filenode, 1

#callback for when the file finishes.
def finished(obj):
	print "Finished playing."

filenode.end_callback = finished

dev.output_object = mixer

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
