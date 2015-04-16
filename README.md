Libaudioverse
==============
Note: this is a work in progress.

##What is Libaudioverse?##

Libaudioverse is three things:

- A library for building realtime audio synthesis applications.

- An implementation of 3D and environmental audio built on top of lower level Libaudioverse components.

- Fast.

###Realtime Audio Synthesis###

Libaudioverse's key concept is the node: an audio processor that has some number of inputs, some number of outputs, and some number of properties.  The outputs of a node can be connected to the inputs of any other node, so long as this does not form a cycle in the graph.  The relationship between inputs and outputs is many to many: you may connect 1 output to as many inputs as you wish, and each input may have as many outputs connected to it as you wish.  Examples of nodes include the sine wave, a lowpass filter, an implementation of HRTF panning, and a ring modulator.  The full list of useful nodes is only growing as time passes, rendering this list far from exhaustive.

To provide a more helpful analogy, it is possible to view Libaudioverse nodes as black boxes with knobs, input jacks, and output wires.  The knobs take the place of properties, changing something about the audio.  The input jacks are the audio to be changed.  The output wires send your transformed audio onward.  Some nodes don't have inputs, instead performing functions like reading files.

This renders Libaudioverse capable of representing any system that can run in constant memory and which does not change the rate of audio.  This includes at least causal linear systems and waveshaping.  If Libaudioverse does not provide a node you need, it is possible to implement it yourself; alternatively, report an issue here or e-mail me and I will see what I can do about adding it to the core library.

###The 3D Simulation###

The 3D simulation supports HRTF via the HRTF panner as well as all common speaker layouts.  It works very similarly to OpenAL or OpenGL.  It consists of two components:

- The environment node specifies information about the environment and the listener.  At the moment, only a "simple" environment without reverb has been implemented, but more are coming.  The environment node outputs audio for all sources which are using it.  It is not possible to change a source's environment, and it is expected that most apps will only ever use one.

- The source node is an input to the environment.  Source nodes have positions in world coordinates and are automatically panned as appropriate.  Unlike other systems, a source node does not work with buffers; instead, you may connect any Libaudioverse node to it.

The two missing features are Doppler and reverb.  These will be implemented before the 1.0 version of this library.

###Fast###

Here is some output from the profiling test for the 3D audio simulation that speaks for itself.  This was taken on my MacBook Pro with an Intel I7 at 2.9 GHZ.  This test is using the built-in HRTF; this means 2 128-point convolutions per source at 44100 KHZ.

~~~
Running 50 times with 250 sources
Took 0.959000 seconds
Estimate: 303.125854 sources maximum
~~~

Note that Libaudioverse does not yet use multiple cores, though it does use SSE2 if compiled with that option.

The above number is the absolute maximum this test thinks it can play in realtime.  For actual realtime playback, expect to top out around 20 sources lower.  For a game or interactive application, halve this number: Libaudioverse can output this many sources, but must hold a lock that prevents changing properties on Libaudioverse objects while it is in the middle of synthesizing the next chunk of audio to be sent to the sound card.  This essentially means that you should be able to get 100 sources on a comparable computer.  If we switch from HRTF panning to normal panning, the test becomes meaningless--if you could max out Libaudioverse on a desktop in such a configuration, you'd have an order of magnitude too many for the player to be able to appreciate.

Optimization is still ongoing, so expect the final version to be faster yet.

##Building##

Currently Libaudioverse only builds on Windows.  This is  a restriction that I intend to lift by the end of May.  Libaudioverse itself will build fine on other platforms, but the build scripts don't know how to handle it and Libaudioverse is currently missing audio backends.  If you are on Linux or Mac, watch this space.

You need the following:

- Python 2.7
- CMake 3.0 or later.
- The python packages Jinja2, pycparser, enum34
- A C++11 capable compiler.  For Windows, this must be Visual Studio 2013.

The build process is the normal build process for CMake, with one small note.  You must build the library as an out-of-source build in a directory called build.  All CMake generators should work.  As an example, here is my build process:

~~~
mkdir build
cd build
CMake -G "NMake Makefiles JOM" ..
jom
~~~

##Bindings##

At the moment, Python and C are the only supported languages.  The above build process will generate Python bindings in build/bindings/python which may be installed in the usual manner.

Libaudioverse's approach to bindings is such that it is possible to add more languages in short order.  If you are seriously considering using Libaudioverse in a specific language, I wish to talk to you.  The addition of a new language is mostly a one-time process, after which the bindings literally maintain themselves.

Note: Your language must support C callbacks, at least 2 levels of pointer indirection and thread primitives in order to be successfully bound to Libaudioverse. The only language I am currently aware of that fails to implement these three things is Angelscript in the BGT scripting environment.
