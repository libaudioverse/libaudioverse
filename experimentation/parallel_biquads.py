import libaudioverse
import math
libaudioverse.initialize()
sim=libaudioverse.Simulation()
count = 5
q = 500
frequency_range = 700
frequency_start = 500
type = libaudioverse.BiquadTypes.peaking

output_mixer = libaudioverse.Mixer(sim, max_parents = count, inputs_per_parent = 2)
f=libaudioverse.File(sim, "footsteps.wav")

def make_filters():
	l=[libaudioverse.Biquad(sim, 1) for i in xrange(count)]
	for c, i in enumerate(l):
		i.filter_tuype = type
		i.frequency = (c+1)*frequency_range/float(count)+frequency_start
		i.q = q
		i.mul = 1.0/count
		output_mixer.inputs[c*2]=i, 0
		output_mixer.inputs[c*2+1] = i, 0
		i.inputs[0]=f, 0
	return l

filters = make_filters()
sim.output_object=output_mixer