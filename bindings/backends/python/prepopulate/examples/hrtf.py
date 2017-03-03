#demonstrates the HRTF panner.
import libaudioverse
import os.path
import time

libaudioverse.initialize()

server = libaudioverse.Server()
server.set_output_device()

print("Enter a file path.")
path = input()
path = os.path.abspath(path)
fnode = libaudioverse.BufferNode(server)
buffer=libaudioverse.Buffer(server)
buffer.load_from_file(path)
fnode.buffer = buffer
fnode.looping.value = True
panner = libaudioverse.HrtfNode(server, "default")
fnode.connect(0, panner, 0)
panner.connect(0, panner.server)

print("""Beginning evaluation.
Enter any python expression that returns a tuple of numbers.  The first is azimuth and the second is elevation.  Azimuth may be anything, but elevation must be on the range -90 to 90.
Enter quit to quit.
""")
while True:
    command = input()
    if command == 'quit':
        break
    az, elev = eval(command)
    panner.azimuth.value = az
    panner.elevation.value = elev

libaudioverse.shutdown()