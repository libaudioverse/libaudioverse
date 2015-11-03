from libaudioverse import *
import math
import itertools
initialize()
s=Simulation(block_size = 100)
n=BlitNode(s)
n.connect_simulation(0)
n.frequency=17
n.should_normalize=False
absolute_error = 0
for i in range(0, 10000, 13):
    integral = 0
    n.frequency = i
    n.reset()
    for j in range(440):
        integral += sum(s.get_block(1))/44100
    if abs(integral-i) > 0.2:
        print("Expected", i, "got", integral)
    absolute_error += abs(integral-i)
print("Absolute error:", absolute_error)