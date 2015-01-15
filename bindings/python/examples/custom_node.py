#Implements a ringmod using the custom object.
import libaudioverse
import time

libaudioverse.initialize()
sim = libaudioverse.Simulation()
def ringmod(obj, frames, input_count, inputs, output_count, outputs):
	for i in xrange(frames):
		outputs[0][i] = inputs[0][i]*inputs[1][i]

ringmod_node= libaudioverse.CustomNode(sim, 2, 1)
ringmod_node.set_processing_callback(ringmod)

w1=libaudioverse.SineNode(sim)
w2=libaudioverse.SineNode(sim)
w1.frequency = 300
w2.frequency = 20
ringmod_node.inputs[0] = w1, 0
ringmod_node.inputs[1] = w2, 0
sim.output_node= ringmod_node

time.sleep(5.0)