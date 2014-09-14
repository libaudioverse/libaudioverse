import libaudioverse
import os.path
import time
time_per_demo = 5.0

libaudioverse.initialize()
sim = libaudioverse.Simulation(device_index = -1)
print """Biquad demo.
Please enter the path to a file in a format supported by Libsndfile: typically wave or ogg.
Mono or stereo files will work fine.  Surround sound files will have additional channels stripped, keeping only the first two."""
filepath = raw_input()
filepath = os.path.abspath(filepath)
filenode = libaudioverse.FileObject(sim, filepath)
bq = libaudioverse.BiquadObject(sim, 2)
if filenode.output_count == 1:
	bq.inputs[0] = filenode, 0
	bq.inputs[1] = filenode, 0
else:
	bq.inputs[0] = filenode, 0
	bq.inputs[1] = filenode, 1
sim.output_object = bq

print "Sweeping lowpass from 0.0 hz to 10000 hz over", time_per_demo, "seconds:"
resolution = 10
for i in xrange(10000/resolution):
	bq.frequency = i*resolution
	time.sleep(time_per_demo/(10000/resolution))

print "Sweeping Highpass from 0 to 10000 over", time_per_demo, "seconds:"
bq.filter_type = libaudioverse.BiquadTypes.highpass
for i in xrange(10000/resolution):
	bq.frequency = i
	time.sleep(time_per_demo/(10000/resolution))

print "Sweeping bandpass from 2000 to 12000 with q=1 over", time_per_demo, "seconds:"
bq.filter_type = libaudioverse.BiquadTypes.bandpass
bq.q = 1
for i in xrange(10000/resolution):
	bq.frequency = 2000+i*resolution
	time.sleep(time_per_demo/(10000/resolution))

print "Sweeping peaking filter from 2000 to 22000 with q=3 and dbgain=-36 over", time_per_demo, "seconds"
bq.filter_type = libaudioverse.BiquadTypes.peaking
bq.q = 3
bq.dbgain = -36
for i in xrange(10000/resolution):
	bq.frequency = 2000+i*resolution*2
	time.sleep(time_per_demo/(10000/resolution))


libaudioverse.shutdown()