from make_cffi import *
ringsize = 360 #how big the ring is.
iterations = 3 #the amount of times to go around in each direction.
inputs = [i for i in xrange(iterations*ringsize)] + [-i for i in xrange(1, iterations*ringsize, 1)]
outputs = [i%ringsize for i in inputs]

def test_ringmod():
	global inputs, outputs, ringsize
	for i, j in zip(inputs, outputs):
		assert lav.ringmod(i, ringsize) == j

def test_ringmodf():
	global inputs, outputs, ringsize
	for i, j in zip(inputs, outputs):
		assert lav.ringmodf(i, ringsize) == j

def test_ringmodi():
	global inputs, outputs, ringsize
	for i, j in zip(inputs, outputs):
		assert lav.ringmodi(i, ringsize) == j
