import libaudioverse
import os.path
import time
time_per_demo = 5.0

libaudioverse.initialize()
sim = libaudioverse.Simulation()
sim.set_output_device()
print("""Biquad demo.
Please enter the path to a file in a format supported by Libsndfile: typically wave or ogg.""")
filepath = input()
filepath = os.path.abspath(filepath)
filenode = libaudioverse.BufferNode(sim)
buffer = libaudioverse.Buffer(sim)
buffer.load_from_file(filepath)
filenode.buffer = buffer
bq = libaudioverse.BiquadNode(sim, 2)
filenode.connect(0, bq, 0)
bq.connect_simulation(0)

print("Sweeping lowpass from 0.0 hz to 10000 hz over", time_per_demo, "seconds:")
resolution = 10
for i in range(10000//resolution):
    bq.frequency.value = i*resolution
    time.sleep(time_per_demo/(10000/resolution))

print("Sweeping Highpass from 0 to 10000 over", time_per_demo, "seconds:")
bq.filter_type.value = libaudioverse.BiquadTypes.highpass
for i in range(10000//resolution):
    bq.frequency.value = i
    time.sleep(time_per_demo/(10000/resolution))

print("Sweeping bandpass from 2000 to 12000 with q=1 over", time_per_demo, "seconds:")
bq.filter_type.value = libaudioverse.BiquadTypes.bandpass
bq.q.value = 1
for i in range(10000//resolution):
    bq.frequency.value = 2000+i*resolution
    time.sleep(time_per_demo/(10000/resolution))

print("Sweeping peaking filter from 2000 to 22000 with q=3 and dbgain=-36 over", time_per_demo, "seconds")
bq.filter_type.value = libaudioverse.BiquadTypes.peaking
bq.q.value = 3
bq.dbgain.value = -36
for i in range(10000//resolution):
    bq.frequency.value = 2000+i*resolution*2
    time.sleep(time_per_demo/(10000/resolution))


libaudioverse.shutdown()