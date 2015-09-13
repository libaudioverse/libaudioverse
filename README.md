Libaudioverse
==============

##Introduction##

Libaudioverse is a highly flexible realtime audio synthesis library.
Potential applications include games, realtime music synthesis, voice chat, implementations of WebAudio, and more.
Libaudioverse supports the best possible backends it can for each platform, and uses both SSE2 and threads for increased performance.

At the core of Libaudioverse is the concept of a node,  a piece of meaningful audio architecture.
They can be connected in any acyclic configuration, allowing the creation of much more complex effects.
It is possible to schedule property changes and envelopes with sample-perfect accuracy;.
For more complex effects, nodes can be connected directly to the properties of other nodes.

here is an overview of the offered nodes:

- The environment and source nodes come together to act as a fully functional 3D audio environment, including support for HRTF, surround sound, and reverb.
- The FDN reverberator is an environmental reverb capable of representing everything from a bathroom to a cathedral.
- If you want to play with Schroeder Allpass sections, try the nested Allpass Network node.
- A variety of lower-level filters are available: Biquad, first-order, one-pole, and convolution.
- It is possible to implement any IIR filter, either by cascading lower level filters or by using the IIR filter node directly.
- Oscillator options include sine and square, as well as a configurable noise generator.
- there are several delay line types.  Most delay lines offer support for feedback, and the filtered delay line allows filtering this feedback.
- You can record audio with the recorder, or intercept audio anywhere in the graph of nodes with the graph listener.
- Finally, if none of these meet your needs, it is possible to create your own node via the custom node.


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
