import libaudioverse
import math
import time

#build binaural beats: two sine waves slightly out of tune, one to the left channel and one to the right.
l = [math.sin((i/44100.0)*2*math.pi*300) for i in xrange(44100)]
r = [math.sin((i/44100.0)*2*math.pi*310) for i in xrange(44100)]
stereo = [i for l in zip(l, r) for i in l]
segment = 2048
index = 0

sim = libaudioverse.Simulation(device_index = -1, block_size = 512, mix_ahead = 10)
p = libaudioverse.PushObject(sim, 44100, 2)
sim.output_object = p

def feeder(obj):
	global segment, index
	obj.feed(segment, stereo[index*segment:(index+1)*segment])
	index += 1
	if index*segment > len(stereo):
		index = 0

p.audio_callback = feeder
time.sleep(10.0)