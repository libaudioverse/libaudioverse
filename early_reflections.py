import gameobjects.matrix44 as mat44
import ode
import libaudioverse
import random
import math
import time
count = 30
space = ode.HashSpace()
rays = [ode.GeomRay(space=space, rlen=100) for i in xrange(count)]
box = ode.GeomBox(space=space, lengths=(30, 30, 1))
corridors = [ode.GeomBox(space=space, lengths=(10, 10, 1)) for i in xrange(4)]
libaudioverse.initialize()
sim = libaudioverse.Simulation()
panners = [libaudioverse.Hrtf(sim, "default") for i in xrange(count)]
delays = [libaudioverse.Delay(sim, max_delay = 20.0, line_count = 1) for i in xrange(count)]
filters = [libaudioverse.Biquad(sim, 1) for i in xrange(count)]
mixer = libaudioverse.Mixer(sim, max_parents = count+1, inputs_per_parent = 2)
f = libaudioverse.File(sim, "footsteps.ogg")
fdn = libaudioverse.FeedbackDelayNetwork(sim, 5.0, count)

#first, set up the panners and the mixer.
for i in xrange(count):
	mixer.inputs[2*i] = panners[i], 0
	mixer.inputs[2*i+1] = panners[i], 1
	panners[i].inputs[0] = fdn, i
	fdn.inputs[i] = delays[i], 0
	delays[i].inputs[0]=filters[i], 0
	filters[i].inputs[0] = f, 0
	filters[i].q=0.2

mixer.inputs[2*count] = f, 0
mixer.inputs[2*count+1] = f, 0

#set up the coridors.
corridors[0].setPosition((7.5, 7.5, 0))
corridors[1].setPosition((-7.5, 7.5, 0))
corridors[2].setPosition((7.5, -7.5, 0))
corridors[3].setPosition((-7.5, -7.5, 0))

#we need to fan out the ODE rays.
def compute_rotation(theta):
	r1=mat44.Matrix44.x_rotation(math.pi/2.0)
	r2 = mat44.Matrix44.z_rotation(theta)
	f= r1*r2
	return f.get_row(0)[:3]+f.get_row(1)[:3]+f.get_row(2)[:3]

#we are facing positive y, our head is positive z, and our right is positive x.
feedback=0
freq_start = 500
freq_range = 1000
freq_q = 0.6
def update(theta, sos=443.0, pos=(0,0,0)):
	for i, v in enumerate(rays):
		angle = (2*math.pi*i/float(count)+theta)
		v.set(pos, (math.sin(angle), math.cos(angle), 0))
	positions = [None]*count
	ray_lengths=[None]*count
	for i in xrange(count):
		collision=ode.collide(box, rays[i])+\
		ode.collide(corridors[0], rays[i])+ode.collide(corridors[1], rays[i])+\
		ode.collide(corridors[2], rays[i])+ode.collide(corridors[3], rays[i])
		if len(collision) == 0:
			length = 1e6 #something absurdly large.
			positions[i] = None
		else:
			possibilities = [j.getContactGeomParams()[0] for j in collision]
			distances = [math.sqrt(sum((j[k]-pos[k])**2 for k in xrange(3))) for j in possibilities]
			needed_index = min([(j[1], j[0]) for j in enumerate(distances)])[1]
			positions[i] = possibilities[needed_index]
			length = distances[needed_index]
		ray_lengths[i]=length if length > 0 else 0.01
	times=[i/float(sos) for i in ray_lengths]
	for time, delay, length, panner, i in zip(times, delays, ray_lengths, panners, xrange(count)):
		delay.delay = min(time, 9.0)
		delay.delay+=delay.delay*.20*random.random()
		delay.mul = min(1.0/length, 1.0)
		delay.feedback=feedback
		filters[i].frequency = freq_start+freq_range*delay.mul
		filters[i].q=freq_q
		panner.azimuth = 360.0*i/count
	#we must now compute and update the fdn feedback matrix and delays.
	new_feedbacks = [0.0]*(count**2)
	new_delays = [0.0]*count
	for i in xrange(count):
		#we calculate the distance between the points, divide by the speed of sound, and use that as the delay.
		p1 = positions[i]
		p2=positions[(i+1)%count]
		if p1 is not None and p2 is not None:
			dist = math.sqrt(sum([(p2[j]-p1[j])**2 for j in xrange(3)]))
		else:
			dist = 0.0
		if dist == 0:
			dist = 0.01
		new_delays[i] = dist/sos #same algorithm as the initial lines.
		new_feedbacks[i*count+((i+1)%count)]=min(1/dist**2, 0.4)
#		new_feedbacks[i*count+(i-1)%count] = min(1/dist**2, 0.4)
	fdn.set_delays(len(new_delays), new_delays)
	fdn.set_feedback_matrix(len(new_feedbacks), new_feedbacks)
	fdn.set_output_gains(count, [1.0/count]*count)

update(0)
sim.output_object = mixer
f.looping=True

def do_walk(t, speed, mul=1):
	stage_length = t/4.0
	count = 10
	print "stage 1"
	for i in xrange(10):
		update(0, pos=(-mul, -mul+2*mul*i/float(count), 0), sos=speed)
		time.sleep(stage_length/10.0)
	print "stage 2"
	for i in xrange(10):
		update(math.pi/2, pos = (-mul+2*i*mul/float(count), mul, 0), sos=speed)
		time.sleep(stage_length/10.0)
	print "stage 3"
	for i in xrange(10):
		update(math.pi, pos=(mul, mul-2*mul*i/float(count), 0), sos=speed)
		time.sleep(stage_length/10.0)
	print "stage 4"
	for i in xrange(10):
		update(3*math.pi/2.0, pos=(mul-2*i*mul/float(count), -mul, 0), sos=speed)
		time.sleep(stage_length/10.0)
	print "returning"
	update(0)

def rotate_about(pos, t, sos):
	for i in xrange(200):
		update(2*math.pi*i/200.0, pos=pos, sos=sos)
		time.sleep(t/200.0)
