Libaudioverse
==============

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

The 3D simulation supports HRTF via the HRTF panner as well as all common speaker layouts.  It works very similarly to OpenAL or OpenGL.  It consists of a few components:

- The environment node specifies information about the environment and the listener.  The environment node outputs audio for all sources which are using it.  It is not possible to change a source's environment, and it is expected that most apps will only ever use one.

- The source node is an input to the environment.  Source nodes have positions in world coordinates and are automatically panned as appropriate.  Unlike some alternatives to Libaudioverse, a source node does not work with buffers; instead, you may connect any Libaudioverse node to it.

- Any number of effects.  An environment may be configured to have "effect sends".  An effect send is an aggregate of all sources connected to it.  Sources may be configured to be connected to specific effect sends, and you get as many effect sends as you want.  These expose themselves as additional outputs on the environment which may be routed through such things as environmental reverbs.

The only currently implemented effect is an environmental reverb, though other effects are still pending.  The envvironmental reverb can be controlled through room density, the time it takes the reverb to decay by 60 decibals, and a few other parameters.
Any node can effectively be used as an effect, and Libaudioverse is flexible enough to let you build your own (just configure a network of nodes and connect an output from the environment to the right place).

###Fast###

Here is some output from Libaudioverse's profiler that speaks for itself.  This output was taken on a Macbook Pro with an Intel I7-3520M at 2.9 GHZ using a Libaudioverse built with VC++ 2013:

~~~
Running profile tests on 2 threads
Estimate for sine nodes: 6344.249512
Estimate for crossfading delay line nodes: 4517.500977
Estimate for biquad nodes: 3594.419922
Estimate for One-pole filter nodes: 4499.990723
Estimate for 2-channel 2-input crossfader nodes: 2502.150391
Estimate for amplitude panner nodes: 4758.187012
Estimate for HRTF panner nodes: 657.168518
Estimate for hard limiter nodes: 4364.653320
Estimate for channel splitter nodes: 4758.187012
Estimate for channel merger nodes: 4003.439941
Estimate for noise nodes: 879.543762
Estimate for square nodes: 2379.093506
Estimate for ringmod nodes: 8862.577148
Estimate for 16x16 FDN nodes: 45.529320
Estimate for 32x32 FDN nodes: 21.302710
~~~

And the following is profiling of the 3D simulation components.  This test estimates the maximum number of sources you can possibly run in realtime:

~~~
Running 50 times with 250 sources on 2 threads
Took 0.848000 seconds
Estimate: 342.803833 sources maximum
~~~

As implied by the output of the above tests, Libaudioverse is capable of using multiple cores to a noticeable advantage.  In addition, Libaudioverse uses SSE2 by default (this can be disabled with a CMake option).

##Building##

Currently Libaudioverse only builds on Windows.  Libaudioverse itself will build with minor changes on other platforms, but the build scripts don't know how to handle it and Libaudioverse is currently missing audio backends.  If you are on Linux or Mac, watch this space; it will build on your platform before 1.0 is released.

You need the following:

- Python 2.7
- CMake 3.0 or later.
- The python packages Jinja2, pycparser, enum34, numpy, and scipy.
- A C++11 capable compiler.  For Windows, this must be Visual Studio 2013 or later.  Building with Visual Studio 2015 gives a decent performance boost.

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
