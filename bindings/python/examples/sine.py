import libaudioverse
import time

#initialize libaudioverse.
libaudioverse.initialize()

#make a device using the default (always stereo) output.
sim = libaudioverse.Simulation()
sim.set_output_device()
#make a sine node.
sine_node = libaudioverse.SineNode(sim)

sine_node.frequency.value = 440

sine_node.connect_simulation(0)

time.sleep(5.0)

libaudioverse.shutdown()
