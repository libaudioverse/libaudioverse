#This script tests operator overloading.
#To run it, you need a sound.wav of at least 10 seconds  in the same directory.

from libaudioverse import *
import time
initialize()
s=Simulation()

def test(p):
    print(p + 5)
    print(5 + p)
    print(p - 3)
    print(3 - p)
    print(p * 2)
    print(2 * p)
    print(p /5)
    print(5 / p)
    print(- p)
    print(float(p))
    print(int(p))
    print(complex(p))
    print(p % 5)
    print(5 % p)
    print(-p)
    print(round(p))
    print(math.floor(p))
    print(math.trunc(p))
    print(math.ceil(p))

n=SineNode(s)
print("Testing float property.")
test(n.frequency)
n.frequency += 5
n.frequency -= 5
n.frequency *= 2
n.frequency /= 2

n = BufferNode(s)
b = Buffer(s)
b.load_from_file("sound.wav")
n.buffer = b
n.position = 1
print("Testing a double property.")
test(n.position)
n.position += 1
n.position -= 1
n.position *= 2
n.position /= 2

n = BlitNode(s)
n.harmonics = 1
print("Testing an int property.")
test(n.harmonics)
n.harmonics += 1
n.harmonics -= 1
n.harmonics *= 2
n.harmonics /= 2
