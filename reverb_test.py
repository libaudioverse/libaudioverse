import random
import math
import libaudioverse
import time
import sys
import wave
import struct
block_frames=1024
libaudioverse.initialize()
sim=libaudioverse.Simulation(device_index = -1 if len(sys.argv)==1 else None, block_size = block_frames)
f=libaudioverse.File(sim, "footsteps.wav")
points = 36
feedback=0.4
feedback_delay=0.02
initial_delay=0.005
initial_delay_range=0.1
initial_delay_periods = 5
mixer=libaudioverse.Mixer(sim, max_parents=points+1, inputs_per_parent=8)
spread=1.0
direction = 0.0
spin_rate = 0.0
spin_rate_random = 0.0

def make_delay_pair():
	d1=libaudioverse.Delay(sim, max_delay=1.0, line_count=1)
	d2=libaudioverse.Delay(sim, max_delay=1.0, line_count=1)
	d2.inputs[0]=d1, 0
	return (d1, d2)

def make_delay_list(count):
	l=[]
	for i in xrange(count):
		pair = make_delay_pair()
		pair[0].inputs[0]=f, 0
		pair[0].delay = initial_delay+random.random()*initial_delay_range
		pair[1].delay=feedback_delay
		pair[1].feedback=feedback
		l.append(pair)
	return l

def make_panner_list(count, l):
	panners=[]
	for i in xrange(count):
		angle=direction+spread*360.0*float(i)/float(count)-spread*360.0/2.0
		panner=libaudioverse.Multipanner(sim, "default")
		panner.azimuth=angle
		panner.inputs[0]=l[i][1], 0
		for j in xrange(8):
			mixer.inputs[i*8+j] = panner, j
		panner.mul = 1.0/count
		panner.strategy=libaudioverse.PanningStrategies.hrtf
		panners.append((l[i][0], l[i][1], panner))
	return panners

reverb = make_panner_list(points, make_delay_list(points))
for i in xrange(8):
	mixer.inputs[points*8+i]=f, 0
sim.output_object=mixer
f.looping = True

if len(sys.argv)==1:
	while True:
		for c, i in enumerate(reverb):
			i[2].azimuth += (spin_rate+spin_rate_random*random.random())*(1 if c%2 else -1)
		time.sleep(0.05)
else:
	output = wave.open(sys.argv[1], "w")
	output.setnchannels(2)
	output.setsampwidth(2)
	output.setframerate(44100)
	written_frames = 0
	while written_frames < float(sys.argv[2])*44100:
		block = sim.get_block(2, False)
		block = [int(32767)*i for i in block]
		block_len = len(block)
		block_data = struct.pack(str(block_len)+"h", *block)
		output.writeframes(block_data)
		written_frames+=block_len/2
	output.close()
