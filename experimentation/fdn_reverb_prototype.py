import libaudioverse
import numpy
libaudioverse.initialize()
s = libaudioverse.Simulation()
s.set_output_device(-1)

b=libaudioverse.Buffer(s)
b.load_from_file("footsteps.wav")
n=libaudioverse.BufferNode(s)
n.buffer=b
n.looping=True

householder_matrix = numpy.identity(8, dtype=numpy.float32)
vector = numpy.array([1.0]*8)
vector.shape = (1, 8)
subtract = 0.25*vector*vector.transpose()
householder_matrix -= subtract
delays = [
2473.0/44100.0,
2767.0/44100.0,
3217.0/44100.0,
3557.0/44100.0,
3907/44100.0,
4127.0/44100.0,
2143.0/44100.0,
1933.0/44100.0,
]

class Reverb(object):
	def __init__(self, simulation):
		self.simulation = simulation
		self.fdn = libaudioverse.FeedbackDelayNetworkNode(self.simulation,  max_delay = 1.0, channels = 8)
		self.output_node = libaudioverse.GainNode(self.simulation, channels = 2)
		self.input_node = libaudioverse.GainNode(self.simulation, channels = 2)
		self.splitter = libaudioverse.ChannelSplitterNode(self.simulation, channels = 2)
		self.merger = libaudioverse.ChannelMergerNode(self.simulation, channels = 2)
		#Set up the graph.
		self.input_node.connect(0, self.splitter, 0)
		self.merger.connect(0, self.output_node, 0)
		for i in xrange(8):
			self.splitter.connect(i%2, self.fdn, i)
			self.fdn.connect(i, self.merger, i%2)
		self.fdn.delays = delays
		self.fdn.filter_types = [libaudioverse.FdnFilterTypes.lowpass]*8
		#Setting these properties makes the FDN configure itself.
		self.feedback_gain = 0.5
		self.cutoff_frequency = 5000

	@property
	def feedback_gain(self):
		return self._feedback_gain
	
	@feedback_gain.setter
	def feedback_gain(self, val):
		mat = householder_matrix*val
		self.fdn.matrix = numpy.ravel(mat)
		self._feedback_gain = val

	@property
	def cutoff_frequency(self):
		return self._cutoff_frequency
	
	@cutoff_frequency.setter
	def cutoff_frequency(self, val):
		self.fdn.filter_frequencies = [val]*8
		self._cutoff_frequency = val

	@property
	def mul(self):
		return self.output_node.mul
		
	@mul.setter
	def mul(self, val):
		self.output_node.mul = val
		
	def connect(self, output, other, input):
		self.output_node.connect(output, other, input)
		
	def connect_simulation(self, output):
		self.output_node.connect_simulation(output)
	
	def disconnect(self, output):
		self.output_node.disconnect(output)

r = Reverb(s)
r.connect_simulation(0)
n.connect(0, r.input_node, 0)
