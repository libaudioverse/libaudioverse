import libaudioverse
import time

#initialize libaudioverse.
libaudioverse.initialize()

#make a device using the default (always stereo) output.
sim = libaudioverse.Simulation(device_index = -1)
#make a sine object.
sobj = libaudioverse.SineObject(sim)
#we make a two-channel mixer.
#Sine objects have one output, but we need to copy it to two channels.
#This will only compute the sine object's value once, never more.
mixer = libaudioverse.MixerObject(sim, max_parents = 1, inputs_per_parent = 2)
#hook the sine object's output to both parent slots on the mixer.
mixer.inputs[0] = sobj, 0
mixer.inputs[1] = sobj, 0
#frequency to 440 HZ.
sobj.frequency = 440
sim.output_object = mixer
time.sleep(5.0)

libaudioverse.shutdown()
