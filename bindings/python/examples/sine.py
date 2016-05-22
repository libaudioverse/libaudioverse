import libaudioverse
import time

#initialize libaudioverse.
libaudioverse.initialize()

#make a device using the default (always stereo) output.
server = libaudioverse.Server()
server.set_output_device()

#make a sine node.
sine_node = libaudioverse.SineNode(server)

sine_node.frequency = 440

sine_node.connect(0, sine_node.server)

time.sleep(5.0)

libaudioverse.shutdown()
