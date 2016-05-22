#demonstrates how to use the 3d simulation.
import libaudioverse
import collections

libaudioverse.initialize()

server = libaudioverse.Server()
server.set_output_device()

world = libaudioverse.EnvironmentNode(server, "default")
world.panning_strategy = libaudioverse.PanningStrategies.hrtf
source = libaudioverse.SourceNode(server, world)
print("Enter a path to a sound file.")
filepath = input()
n = libaudioverse.BufferNode(server)
b = libaudioverse.Buffer(server)
b.load_from_file(filepath)
n.buffer = b
n.connect(0, source, 0)

n.looping = True

world.connect(0, world.server)

print("""Enter python expressions that evaluate to 3-tuples (x, y, z).
Positive x is to your right, positive y is above you, and positive z is behind you.
Enter quit to quit.""")
while True:
    command = input()
    if command == 'quit':
        break
    vect = eval(command)
    if not isinstance(vect, collections.Sized) or len(vect) != 3:
        print("Must evaluate to a 3-tuple.  Try again")
        continue
    source.position.value = vect

libaudioverse.shutdown()