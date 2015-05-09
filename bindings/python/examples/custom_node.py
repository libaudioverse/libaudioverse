#Implements a ringmod using the custom object.
import libaudioverse
import time

libaudioverse.initialize()
sim = libaudioverse.Simulation()
sim.set_output_device(-1)
def ringmod(obj, frames, input_count, inputs, output_count, outputs):
	for i in xrange(frames):
		outputs[0][i] = inputs[0][i]*inputs[1][i]

ringmod_node= libaudioverse.CustomNode(sim, 2, 1, 1, 1)
ringmod_node.set_processing_callback(ringmod)

w1=libaudioverse.SineNode(sim)
w2=libaudioverse.SineNode(sim)
w1.frequency.value = 30
w2.frequency.value = 300
w1.connect(0, ringmod_node, 0)
w2.connect(0, ringmod_node, 1)

ringmod_node.connect_simulation(0)

time.sleep(5.0)