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

- The library should be comprehensible without doing a ton of research on 3D audio and clearly documented.

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

- Documentation, user's guide, and tutorials.

#Implementation#

##Code style##

This project obeys the following conventions.  Line length is not limited, as I believe meaningful names to be more important.

- Variables are camelcase with the first letter lowercase.  So too are struct members, unless said struct member is a function pointer.

- Functions and struct members that are function pointers are lowercase with underscore (`_`) separating the words.

- Macros are uppercase with underscores between words.

- Typedefs are camelcase with the first letter uppercase.

- Indentation is one tab.

- Opening braces are the last noncomment on the same line as the construct they open.  Closing braces are on a line by themselves.  Blocks that have more than one statement shall be written vertically, not horizontally and all on the same line.

##Achieving Inheritence from Target Languages##

C has a number of tricks.  The first of these that concerns this library is the following: a pointer to type struct may be safely cast to a pointer to the first member of a struct; furthermore, it is only necessary to know the type of this member.  This rule may be applied recursively.

As a demonstration, consider the following:

~~~C
typedef struct {...} a;
typedef struct {a base;...} b;
typedef struct {b base;...} c;
~~~

The following operations are safe:

~~~
b obj1;
c obj2;
//initialization code here...

a *ptr;
ptr = (a*)&obj1; //safe.  ptr points to obj1.base
ptr = (a*)&obj2; //Safe. ptr points to obj2.base.base.
~~~

What this means is that single inheritence may be implemented.  The idea for doing so is a bit like Python.
Require objects to know their bases, and specifically call obj_foo functions to go up the call chain.  The first parameter to every function is the object to act upon.  Because of the above trick, subtypes that declare as their first member their base can be passed in without trouble.

The second portion to inheritance and inheritance from target languages is this: safe for initialization, all calls which operate on publicly visible objects shall act through function pointers.  Through bindings, these objects may be overridden either by using the same trick as above and building a larger struct or replacing function pointers, and making sure to save the old ones somewhere.  How beautiful this may be made depends on how dynamic the language is-I suspect that it can be automated in Python, for example, and that regular Pythonic inheritance can be made to work.

##Description of the Signal Flow Graph##

A signal flow graph specifies what signals go where.  This is the heart of the library.  Signal Flow Graphs are composed of two objects: nodes and edges.

A node is a source, filter, or other such object.  Nodes have some number of inputs, some number of properties, and some number of outputs.  A node termed an input node has only outputs.  A node termed an output node has only inputs.  All other nodes are termed internal nodes.  There may be any number of internal nodes and input nodes, but only one output node.

Nodes are considered publically available objects and thus follow the above guidelines for implementing inheritance.  Edges are not: they are very internal to the library and only properly exist to support feedback.

The nodes which an object can reach by following inputs starting at itself are termed its parents.  The nodes which an object can reach because they are connected directly to its inputs are termed its immediate parents.  No object may be a parent of itself.

The nodes which an node can reach by following outputs beginning at itself are termed its children; the nodes connected directly to an output of a node are its immediate children.  Since no object may be a parent of itself, no object may be a child of itself.  One algorithm fulfills both of these rules.

In terms of the code, nodes represent sounds, modifiers, or outputs.  They all inherit from the `Node` struct.  The inputs of an object point directly at those objects for which they are to read with an offset into the output array.  The outputs of an object are 0 or more buffers with pointers in an outputs array.  In order to support feedback, which is discussed below, every node adds a latency of one sample.  See the header files for the precise interface.

For performance reasons, children are not recorded, only parents.  This connection need not go both ways.  It is enough for an object to know that it has children and to expose outputs; any access to an object's children is effectively looking into an object that is in the middle of processing.

Finally, the mathematical code onf an object is not embodied in the object itself.  It is instead embodied in a kernel, a function which takes all information required to perform an operation, as well as a place to which to write output.

###The processing Algorithm###

1.  At initialization time, the caller specifies a block size: the number of samples to be generated at once.

2. The code responsible for feeding the sound card counts as machine specific; it is outside the signal graph.  When the audio card needs more audio, it asks the output node to compute itself.

3.  The output node increments the tick counter.

4.  The output node calls process on all of its parents, who call process on their parents recursively (but this is through edges, see the notes on feedback below).

5.  When a node has processed all of its parents, it performs its own operation.  It then marks itself as processed for this tick by saving the tick counter.  Further calls to that node's processing function are then no-op until the next tick.

6.  This audio is fed to the sound card.

###Support for recursive graphs###

The above specifies that all nodes shall introduce a minimum latency of one sample.  This means that, should there be a cycle in the audio graph, an object may ask for one sample from its parents before its parents have to begin processing.  This is not implemented in the node code, but rather in the edge-handling code: after each processing interval, the last sample of the buffer is copied to the first, and all processing begins at the second (so a 512 bock size actually creates buffers of a minimum size 513.  The first 512 will be seen this tick, but the last 512 will be what is processed).
To implement feedback, therefore, the buffer size is lowered to one sample.  This section begins with a description of the problem from a programming perspective, as this is both the most difficult part of the library conceptually and quite possibly the most difficult to code.  This latter reason is why it is not at the beginning of the goal list.

Consider hypothetical nodes A and B, who are connected in a simple cycle.  We wish to extract output from b, and feed input to a.  it is trivial to feed the input into a, but not to get the output out of b.  For the purposes of this description, we are at time unit 5, and the notation a[5] means the sample that a produces at time unit 5

Let there be no latency.  A and b are about to process time unit 5.  We ask for some output from B; for this example, it does not matter how much.  B begins processing and calls process on a, with the desire to get a[5].  B has not yet processed to generate B[5].  The problem here is that a needs b[5].  This is either an infinite loop, crash, deadlock, or just nonsense output.

But add a latency of one sample.  To process b[5], b needs a[4].  A needs b[3].  B needs a[2].  A needs b[1].  B[1] needs A[0].

A[0] is within the one-sample latency at the beginning of all processing.  A[0] is therefore 0.  This  allows everything to process.

But here is the problem: calculating everything one sample at a time is incredibly CPu intensive and incredibly cache unfriendly.  Furthermore, stack allocations and deallocations begin taking more time than the math.  If the latency is made larger, the block size can increase, but this introduces distortion and causes unpredictable behavior.  In order to work well enough for games, the library needs to be able to process in chunks that are larger.  To handle this, a feedback node is needed-essentially an internal copy of the library that holds a smaller signal graph.  The feedback node can feel free to lower the block size to 1; when processed, it processes the encapsulated cycle more than once to generate the output samples.  Feedback is important for reverb, as well as other such things; it is still one of the rarer nodes and no more than five or six are likely to ever exist at once.