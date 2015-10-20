Libaudioverse
==============

[![Windows Build status](https://ci.appveyor.com/api/projects/status/wmoa6isbe8fdmg2c?svg=true)](https://ci.appveyor.com/project/camlorn/libaudioverse)

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

- Boost.  I test with 1.59.0, but other versions may work.
- Python 3.5. If you are on Windows, the launcher must be working.
- CMake 3.3 or later.
- The python packages PyYAML, Jinja2, pycparser, enum34, numpy, and scipy.
- A C++11 capable compiler.  For Windows, this must be Visual Studio 2015 or later.

NOTE: Visual Studio 2015 does not include the C++ components by default.
You need to be sure to do a custom install and get at least the C/C++ compiler and all of the Windows headers.

The build process is the normal build process for CMake.
You must build the library as an out-of-source build in a directory called build.
All CMake generators should work.

For generators without multiple build types, the default configuration of a newly created build directory is Release.
On Windows, I suggest using `nmake` from a Visual Studio 2015 developer command prompt; if you choose to use Visual Studio project files, make sure to change your build configuration to release.

If you are not using binaries provided by me and need to build Libaudioverse frequently, my generator of choice is `NMake Makefiles JOMM`.
Jom is a parallel version of NMake and may be obtained from [here](https://wiki.qt.io/Jom).
Unlike MSBuild, Jom is much less verbose.

Here is an example set of commands, assuming you have all components and Jom.
If you are using a different generator, you will need to change the last one.

~~~
mkdir build
cd build
CMake -G "NMake Makefiles JOM" ..
jom
~~~

To generate the manual, you will additionally need  asciidoctor.
Execute the documentation target to generate `build/documentation/libaudioverse_manual.html`, which contains what currently exists of the manual.

##Bindings##

At the moment, Python and C are the only supported languages.  The above build process will generate Python bindings in build/bindings/python which may be installed in the usual manner.

Libaudioverse's approach to bindings is such that it is possible to add more languages in short order.  If you are seriously considering using Libaudioverse in a specific language, I wish to talk to you.  The addition of a new language is mostly a one-time process, after which the bindings literally maintain themselves.

Note: Your language must support C callbacks, at least 2 levels of pointer indirection and thread primitives in order to be successfully bound to Libaudioverse. The only language I am currently aware of that fails to implement these three things is Angelscript in the BGT scripting environment.
