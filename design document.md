#Camlorn_audio2 Design Speciication and notes#

##Introduction##

Camlorn_audio2 is a working title, and will be replaced.  For this reason, and because it is also repetative and a bit hard to type, it is hereafter "the library", "this library", or somesuch.

This document serves three major purposes: to specify the target features for a 1.0 release, to familiarize new contributors with the code, and to serve as a repository for the concepts and interface asumptions used internally in the library. The first section of this document lists goals, features, and objectives; the second portion discusses how the library works, how new components may be added, and how these procedures meet the goals outlined in the first section.

As implementation proceeds, this document may become outdated.  This document's main purpose is to prevent problems as seen in the original Camlorn_audio, i.e. lack of understanding of the problem domain and the required features for adoption.

#Goals and Features#

Briefly, this library aims to be performant, featureful, and flexible.  If a feature is not possible with the current state of the library and is within the realm of reaonable implementation, especially if such features have to do with audio, they should be added.  Goals for all releases are listed below:

- The library should perform fast enough for realtime use.  In simple configurations, realtime use is defined as 32 consecutive playing sources plus one environmental effect.  This definition is needed because it is possible to overtax all audio libraries at some point.

- This library should be accessible from any language with a C FFI.

- It may utilize hardware specific features but shall never depend on them.

- The library should support dynamic configurations of the signal graph.

- The library should allow new nodes in the signal graph to be defined in target languages, with the associated performance penalties or advantages thereof.

- The library should support callbacks as much as possible, but must support at a minimum callbacks when queued audio at input nodes is about to run out, and when aqueued audio at input nodes has run out.

- The library should have some support for feedback, as this is needed for reverb.

##Roadmap for 1.0##

The proposed release schedule is as follows.

First preview, not intended for actual adoption:

- Implements basic moving sources and a listener.

- Full dynamic signal graph manipulation need not be part of the external interface.

- Basic python bindings work.

- node for statically reading a file works, but not streaming.

- No callbacks.

- Feedback is not permitted.

0.1:

This is intended to be the first release with full useful 3d audio functionality.

- The signal graph is still hidden from the user and feedback is still unimplemented.

- Reverb works.

- Callbacks for audio having stopped work, but not those warning it will soon run out.

From here, binaries will be provided as I feel like it, and at relative points of stability until 1.0.

Goals for 1.0 in no particular order:

- begin exposing the signal graph externally and implement the feedback node.  Investigate using this for a better reverb algorithm for headphones.

- Implement streaming from files.

- Complete [clang_helper](http://github.com/camlorn/clang_helper) to the necessary state to begin autogenerating bindings, or opt for swig.  Attempt to provide bindings in at least 3 languages to make sure that this can actually be done.

- Now that streaming sound nodes exist, implement callbacks to warn the user of impending playback ending.  This is useful most notably for queuing dynamic sound generation, such as footsteps as they change terrain.

- Implement the ability for languages to write their own nodes.  Note that this goal is a bit difficult to do well; see the notes on object-oriented programming in C.

#Implementation#

