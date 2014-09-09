import libaudioverse
import math
import time

l = [math.sin((i/44100.0)*2*math.pi*300) for i in xrange(147)]
r = [math.sin((i/44100.0)*2*math.pi*300) for i in xrange(147)]
stereo = [i for l in zip(l, r) for i in l]*20

libaudioverse.initialize()

sim = libaudioverse.Simulation(device_index = -1, block_size = 1024, mix_ahead = 2)
p = libaudioverse.PushObject(sim, 48000, 2)
sim.output_object = p
def audio_callback(obj):
	p.feed(len(stereo), stereo)

def out_callback(obj):
	p.feed(len(stereo), stereo)

p.threshold = 0.1
p.audio_callback = audio_callback
p.out_callback = out_callback
time.sleep(10.0)
libaudioverse.shutdown()