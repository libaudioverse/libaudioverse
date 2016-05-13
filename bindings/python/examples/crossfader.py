import libaudioverse
import threading
libaudioverse.initialize()

server = libaudioverse.Server()
server.set_output_device()

s1 = libaudioverse.SineNode(server)
s1.frequency = 100
s2 = libaudioverse.SineNode(server)
s2.frequency = 200
s3 = libaudioverse.SineNode(server)
s3.frequency = 400

fader = libaudioverse.CrossfaderNode(server, channels = 2, inputs = 3)
s1.connect(0, fader, 0)
s2.connect(0, fader, 1)
s3.connect(0, fader, 2)

fader.connect_server(0)

#Let's crossfade back and fortha cross all the inputs repeatedly.

crossfade_sem = threading.Semaphore(value = 0)

def done(*args):
    crossfade_sem.release()

fader.set_finished_callback(done)

for i in range(5):
    fader.crossfade(duration = 0.5, input = 1)
    crossfade_sem.acquire()
    fader.crossfade(duration = 0.5, input = 2)
    crossfade_sem.acquire()
    fader.crossfade(duration = 0.5, input = 1)
    crossfade_sem.acquire()
    fader.crossfade(duration = 0.5, input = 0)
    crossfade_sem.acquire()

libaudioverse.shutdown()