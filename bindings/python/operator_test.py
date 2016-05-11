#This is a temporary script to test all the possible operators.
#I'm not sure what will happen to this: it may be come unit tests.  Or not. But it will go away.

from libaudioverse import *
import time
initialize()
s=Simulation()

#Test a float property
n=SineNode(s)

print(n.frequency + 5)
print(5 + n.frequency)
print(n.frequency - 3)
print(3 - n.frequency)
print(n.frequency * 2)
print(2 * n.frequency)
print(n.frequency /5)
print(5 / n.frequency)
print(- n.frequency)
print(float(n.frequency))
print(int(n.frequency))
print(complex(n.frequency))
print(n.frequency % 5)
print(5 % n.frequency)
print(-n.frequency)
print(round(n.frequency))
print(math.floor(n.frequency))
print(math.trunc(n.frequency))
print(math.ceil(n.frequency))
