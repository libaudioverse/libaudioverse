import libaudioverse
import math
import time

l = [math.sin((i/44100.0)*2*math.pi*300) for i in xrange(147)]
r = [math.sin((i/44100.0)*2*math.pi*300) for i in xrange(147)]
stereo = [i for l in zip(l, r) for i in l]*20

libaudioverse.initialize()

sim = libaudioverse.Simulation()
sim.set_output_device(-1)
p = libaudioverse.PushNode(sim, 48000, 2)

def audio_callback(obj):
	p.feed(len(stereo), stereo)

def out_callback(obj):
	p.feed(len(stereo), stereo)

p.threshold.value = 0.1
p.audio_event = audio_callback
p.out_event = out_callback

p.connect_simulation(0)

time.sleep(10.0)
libaudioverse.shutdown()