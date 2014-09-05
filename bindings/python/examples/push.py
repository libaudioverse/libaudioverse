import libaudioverse
import math
import time

#build binaural beats: two sine waves slightly out of tune, one to the left channel and one to the right.
l = [math.sin((i/44100.0)*2*math.pi*300) for i in xrange(44100)]
r = [math.sin((i/44100.0)*2*math.pi*310) for i in xrange(44100)]
stereo = [i for l in zip(l, r) for i in l]

sim = libaudioverse.Simulation(device_index = -1)
p = libaudioverse.PushObject(sim, 44100, 2)
sim.output_object = p

def feeder(obj):
	obj.feed(len(stereo), stereo)

p.audio_callback = feeder
time.sleep(10.0)