#a simple command line media player.
import libaudioverse
import os.path
libaudioverse.initialize()

server = libaudioverse.Server()
server.set_output_device()

print("""Command line Media player.
Please enter the path to a file in a format supported by Libsndfile: typically wave or ogg.""")
filepath = input()
filepath = os.path.abspath(filepath)
filenode = libaudioverse.BufferNode(server)
buffer = libaudioverse.Buffer(server)
buffer.load_from_file(filepath)
filenode.buffer = buffer

#callback for when the file finishes.
def finished(obj):
    print("Finished playing.")

filenode.set_end_callback(finished)

filenode.connect(0, filenode.server)

commands = """Commands:
play
pause
rate <number>
seek <seconds>
quit
"""

print(commands)

while True:
    try:
        command = input().split(" ")
        if command[0] == 'quit':
            break
        elif command[0] == 'play':
            filenode.state = libaudioverse.NodeStates.playing
        elif command[0] == 'pause':
            filenode.state = libaudioverse.NodeStates.paused
        elif command[0] == 'seek':
            to = float(command[1])
            filenode.position = to
        elif command[0] == 'rate':
            to = float(command[1])
            filenode.rate = to
    except Exception as e:
        print("Libaudioverse error.  Unrecognized command, or invalid syntax.")
        print(commands)

libaudioverse.shutdown()