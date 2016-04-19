Libaudioverse
==============

[![Windows Build status](https://ci.appveyor.com/api/projects/status/wmoa6isbe8fdmg2c?svg=true)](https://ci.appveyor.com/project/camlorn/libaudioverse)

[![Linux Build Status](https://travis-ci.org/camlorn/libaudioverse.svg?branch=master)](https://travis-ci.org/camlorn/libaudioverse)

[GitHub](http://github.com/camlorn/libaudioverse)

##Introduction##

Libaudioverse is a highly flexible realtime audio synthesis library designed to be bound to as many languages as possible.
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

NOTE: This is pre-alpha and supports Windows and Linux.  Mac is planned.

##Licensing##

Libaudioverse is dual-licernsed under the [Mozilla Public License 2.0](https://www.mozilla.org/en-US/MPL/2.0/) and [the Gnu General Public License, Version 3 or Later](http://www.gnu.org/licenses/gpl-3.0.en.html).
This means that you may use this code under the terms of either license.
Specific information on copyright canb be found in the file `LICENSE`, a copy of the Mozilla Public License version 2.0 in the file `license.mpl`, and a copy of the Gnu General Public License version 3 in `license.gpl`.

##Documentation and Examples##

There are two sources of Libaudioverse documentation.

The first is the [language-agnostic manual](http://camlorn.github.io/libaudioverse/docs/libaudioverse_manual.html), which discusses Libaudioverse from a general perspective.
This manual contains the reference for the C API and an overview of Libaudioverse's core concepts.
Most examples in this manual are in Python.

The second source of documentation is the API reference for your language of choice.
At the moment, this means [the Python API reference](http://camlorn.github.io/libaudioverse/docs/python/index.html).
The API references contain installation instructions and any notes specific to the language in question.

Examples for all supported languages may be found in the [GitHub repository](http://github.com/camlorn/libaudioverse).
These sets of examples aim to be equivalent and to demonstrate most critical features of Libaudioverse.
This library is easy.
In many cases, the examples will be enough to get you started.

##Getting Help##

Libaudioverse has [a Google Group](https://groups.google.com/a/camlorn.net/forum/#!forum/libaudioverse).
You can subscribe directly and without a Gmail address via e-mailing an empty e-mail to `libaudioverse+subscribe@camlorn.net` and clicking the link in the confirmation e-mail sent to you.
I prefer questions to come via this avenue, as it results in your answers being searchable in future.
If you need to contact me in real-time, you can do so via the Libaudioverse IRC channel: `#libaudioverse` on chat.freenode.net.

Please report bugs and make feature requests using the GitHub issue tracker; this saves me time for issues which I cannot fix immediately.

##Building##

See the [info on supported platforms and build instructions](http://github.com/camlorn/libaudioverse/tree/master/platform_support.md).

##Bindings##

At the moment, Python and C are the only supported languages.  You can get the Python bindings via `pip` on Windows, but Linux currently requires building Libaudioverse yourself.  As more languages become available, Libaudioverse  will attempt to upload binaries to package managers.  The goal is to minimize the number of use cases that require building Libaudioverse.

Libaudioverse's approach to bindings is such that it is possible to add more languages in short order.  If you are seriously considering using Libaudioverse in a specific language, I wish to talk to you.  The addition of a new language is mostly a one-time process, after which the bindings literally maintain themselves.  Which language I add next is primarily based on interest.

Note: Your language must support C callbacks, at least 2 levels of pointer indirection and thread primitives in order to be successfully bound to Libaudioverse. The only language I am currently aware of that fails to implement these three things is Angelscript in the BGT scripting environment.
