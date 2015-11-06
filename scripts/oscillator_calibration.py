#This is a quick and hacky test script that was used in the additive oscillators to account for the Gibbs Phenomenon.
#By replacing the node, it's possible to test an oscillator to see if it's not  quite on the range -1 to 1.
from libaudioverse import *
initialize()
s=Simulation(block_size = 100)
n=AdditiveSquareNode(s)
n.connect_simulation(0)
tolerence = 1e-2
for i in range(13, 22000, 13):
    n.reset()
    n.frequency = i
    minimum, maximum = float('INFINITY'), float('-INFINITY')
    for j in range(441):
        b=s.get_block(1)
        minimum = min(minimum, min(b))
        maximum = max(maximum, max(b))
    if abs(1-abs(minimum)) > tolerence or abs(1-abs(maximum)) > tolerence:
        print("frequency",  i, "minimum =", minimum, "maximum =", maximum)
