#Implements a ringmod using the custom object.
import libaudioverse
import time

libaudioverse.initialize()
sim = libaudioverse.Simulation()
def ringmod(obj, frames, input_count, inputs, output_count, outputs):
	for i in xrange(frames):
		outputs[0][i] = inputs[0][i]*inputs[1][i]

ringmod_object = libaudioverse.CustomObject(sim, 2, 1)
ringmod_object.set_processing_callback(ringmod)

w1=libaudioverse.SineObject(sim)
w2=libaudioverse.SineObject(sim)
w1.frequency = 300
w2.frequency = 20
ringmod_object.inputs[0] = w1, 0
ringmod_object.inputs[1] = w2, 0
sim.output_object = ringmod_object

time.sleep(5.0)