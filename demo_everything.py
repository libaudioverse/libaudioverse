import libaudioverse
import math
import time
import random

libaudioverse.initialize()

simulation = libaudioverse.Simulation(block_size=128)

def make_fm(carrier, ratio, index):
    carrier_node = libaudioverse.SineNode(simulation)
    modulator_node =libaudioverse.SineNode(simulation)
    modulator_frequency = ratio*carrier
    modulator_amplitude = index*modulator_frequency
    carrier_node.frequency.value = carrier
    modulator_node.frequency.value = modulator_frequency
    modulator_node.mul.value = modulator_amplitude
    modulator_node.connect_property(0, carrier_node.frequency)
    return carrier_node

fm1 = make_fm(carrier= 50, ratio = 0.5, index = 50)
fm2=make_fm(carrier=90, ratio = 1.03, index=15)

#A bit of noise makes the HRTF much more obvious.
noise=libaudioverse.NoiseNode(simulation)
noise.noise_type.value =libaudioverse.NoiseTypes.pink

fm1.mul.value = 0.1
fm2.mul.value = 0.1
noise.mul.value=0.05

fm1.mul.set(time=28.0, value = 0.1)
fm2.mul.set(time = 28.0, value = 0.1)
noise.mul.set(time=28.0, value=0.05)
fm1.mul.linear_ramp_to_value(time =29.8, value=0.0)
fm2.mul.linear_ramp_to_value(time=29.8, value = 0.0)
noise.mul.linear_ramp_to_value(time = 29.8, value= 0.0)

#pass everything through a ringmod.
rm=libaudioverse.RingmodNode(simulation)
fm1.connect(0, rm, 0)
fm2.connect(0, rm, 0)
noise.connect(0, rm, 0)
modulator=libaudioverse.SineNode(simulation)
modulator.frequency.value = 300
modulator.connect(0, rm, 1)

#The ringmod starts fading out at 30 seconds.
rm.mul.set(time = 28.0, value = 1.0)
rm.mul.linear_ramp_to_value(time = 30.0, value = 0.0)

#This is the 3D infrastructure.
world=libaudioverse.EnvironmentNode(simulation, "default")
world.orientation.value = (0, 1, 0, 0, 0, 1)

#Turn on HRTF.
world.default_panning_strategy.value=libaudioverse.PanningStrategies.hrtf
world.output_channels.value= 2

#Set a default for the distance model.
world.default_max_distance.value = 20

#Get a source.
source=libaudioverse.SourceNode(simulation, environment=world)

rm.connect(0, source, 0)

world.connect_simulation(0)

square_side_length = 15
square_phase_length=5.0

def block_callback(simulation, time):
    phase= (time//square_phase_length)%4
    percent=(time%square_phase_length)/square_phase_length
    if phase==0:
        source.position.value = (-square_side_length/2+square_side_length*percent, square_side_length/2, 0)
    elif phase==1:
        source.position.value = (square_side_length/2, square_side_length/2-square_side_length*percent, 0)
    elif phase==2:
        source.position.value = (square_side_length/2-square_side_length*percent, -square_side_length/2, 0)
    elif phase==3:
        source.position.value = (-square_side_length/2, -square_side_length/2+percent*square_side_length, 0)

simulation.set_block_callback(block_callback)

print("Writing 30 seconds of audio to out.ogg...")
simulation.write_file(path="out.ogg", channels=2, duration =30.0)
#simulation.set_output_device(-1, mixahead=20)
#raw_input()

libaudioverse.shutdown()