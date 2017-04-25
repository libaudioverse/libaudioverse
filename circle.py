from libaudioverse import *
import math
import time
initialize()
s = Server(block_size = 128)
e = EnvironmentNode(s, "default")
o = SourceNode(s, e)
n = BufferNode(s)
b = Buffer(s)
b.load_from_file("sound.wav")
n.buffer = b
n.connect(0, o, 0)
e.connect(0, s)

radius = 3
offset = 0.3

e.position = 0, 0, offset
e.panning_strategy = PanningStrategies.hrtf

angle = 0

#s.set_output_device()

def cb(*args):
    global angle
    angle += math.pi/1024
    angle = angle % (2*math.pi)
    x, y =radius*math.cos(angle), radius*math.sin(angle)
    o.position = x, y, 0

s.set_block_callback(cb)

s.write_file("out.ogg", channels = 2, duration = 10)

shutdown()