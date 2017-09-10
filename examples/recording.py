"""An example of recording with Libaudioverse and SoundDevice."""

try:
    import sounddevice as sd
except ModuleNotFoundError:
    print('Please install sounddevice.')
    raise SystemExit

import libaudioverse as lav

sr = 44100  # Samplerate.
channels = 1  # Let's record in mono.

# Let's set sounddevice defaults so we don't have to supply them to
# sounddevice.rec et all.

sd.default.samplerate = sr
sd.default.channels = channels

lav.initialize()
print('Initialized Libaudioverse.')

server = lav.Server()
server.set_output_device('default')

node = lav.PushNode(server, sr, channels)
node.connect(0, server)

# Create some fx to show off:
reverb = lav.FdnReverbNode(server)
reverb.connect(0, server)
reverb.t60.value = 5.0
node.connect(0, reverb, 0)

delay = lav.CrossfadingDelayNode(server, 2.0, channels)
delay.connect(0, server)
delay.mul.value = 0.1
delay.delay.value = 1.0
delay.feedback.value = 0.2
node.connect(0, delay, 0)
delay.connect(0, reverb, 0)

print('Created fx.')


# The callback for sd.InputStream:
def cb(data, frames, time, status):
    """Feed the node."""
    node.feed(frames, data)


input = sd.InputStream(callback=cb)
input.start()  # Starts in a different thread.

print('Input started.')

if __name__ == '__main__':
    try:
        print('Press Control C to end.')
        while True:
            continue  # Make a main loop.
    except KeyboardInterrupt:
        print('Shutting down.')
    lav.shutdown()
