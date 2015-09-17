from __future__ import division
from libaudioverse import *
import math

initialize()
s=Simulation()
n=BufferNode(s)
b=Buffer(s)
b.load_from_file("sound.wav")
n.buffer.value=b
n.looping.value = True

n.mul.value = 0.0
n.mul.linear_ramp_to_value(0.1, 1.0)
n.position.value = 40
world = SimpleEnvironmentNode(s, "default")
world.default_panning_strategy.value= PanningStrategies.hrtf
source = SourceNode(s, world)
n.connect(0, source, 0)

circles = 10
distance = 8
duration = 60
min_elev = -10
max_elev = 10
elev_delta = max_elev-min_elev

world.orientation.value = (0, 1, 0, 0, 0, 1)

def block_callback(simulation, time):
    height = min_elev+(time/duration)*elev_delta
    radians = 2*math.pi*circles*time/duration
    x = distance*math.sin(radians)
    y = distance*math.cos(radians)
    source.position.value = (x, y, height)

world.connect_simulation(0)
s.set_block_callback(block_callback)
s.write_file("out.ogg", channels = 2, duration = duration)