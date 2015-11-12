#a simple command line media player.
import libaudioverse
import os.path
libaudioverse.initialize()

sim = libaudioverse.Simulation()
sim.set_output_device(-1)
print """Command line Media player.
Please enter the path to a file in a format supported by Libsndfile: typically wave or ogg."""
filepath = raw_input()
filepath = os.path.abspath(filepath)
filenode = libaudioverse.BufferNode(sim)
buffer = libaudioverse.Buffer(sim)
buffer.load_from_file(filepath)
filenode.buffer = buffer

#callback for when the file finishes.
def finished(obj):
    print "Finished playing."

filenode.end_event = finished

filenode.connect_simulation(0)

commands = """Commands:
play
pause
rate <number>
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
            filenode.state.value = libaudioverse.NodeStates.playing
        elif command[0] == 'pause':
            filenode.state.value = libaudioverse.NodeStates.paused
        elif command[0] == 'seek':
            to = float(command[1])
            filenode.position.value = to
        elif command[0] == 'rate':
            to = float(command[1])
            filenode.rate.value = to
    except Exception as e:
        print "Libaudioverse error.  Unrecognized command, or invalid syntax."
        print commands

libaudioverse.shutdown()