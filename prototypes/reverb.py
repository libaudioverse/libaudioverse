#This file is the test harness/prototype needed to run the late reflections node in a minimal configuration.
from libaudioverse import *
import math
import random
order=16
initialize()
s=Simulation()

n=BufferNode(s)
b=Buffer(s)
b.load_from_file("footsteps.wav")
n.set_buffer(b)


main_panner_map = [360.0*i/order for i in xrange(order)]

main_panner=AmplitudePannerNode(s)
main_panner.channel_map = main_panner_map
main_panner.skip_lfe=False
main_panner.skip_center=False

splitter = ChannelSplitterNode(s, order)

n.connect(0, main_panner, 0)
main_panner.connect(0, splitter, 0)

panner_bank = PannerBankNode(s, panner_count = order+1, hrtf_path = "default")

late = LateReflectionsNode(s)

for i in xrange(order):
	splitter.connect(i, late, i)
	late.connect(i, panner_bank, i)

panner_bank.connect_simulation(0)
panner_bank.distance=30
panner_bank.spread=360
panner_bank.strategy=PanningStrategies.hrtf


s.set_output_device(-1)