Libaudioverse
==============
Note: this is a work in progress.

Libaudioverse is a library for 3D and environmental audio.  It provides abstractions for most audio tasks through a system of interconnected objects and bindings to multiple programming languages.  The goals of Libaudioverse are to provide a practical platform for implementing synthesis techniques researched elsewhere, a library for real-time audio output and manipulation, and an easy to use environmental audio model.

Libaudioverse objects are black boxes with some number of inputs, outputs, and properties.  Each object takes its inputs, applies an algorithm to them, and produces one or more outputs.  Exactly one object is the output object, whose outputs are connected directly to the speaker system.  There are 3 key restrictions to this model which stem from the goal of building complex environmental simulations that can play in real-time.  These restrictions give something along the lines of a 10x performance boost, while leaving plenty of flexibility to implement most models.

- The graph representing the connections between objects may never contain a cycle.  Note that cyclic algorithms may still be implemented, but must be contained in a single object.

- Objects must output as many samples as they receive.

- Properties may not be connected to outputs from other objects.

Concrete examples of Libaudioverse objects include these: file reading, wave synthesis, HRTF panning, delay lines with feedback, limiters, and attenuators.  The 3d audio simulation is itself built on top of source and environment objects.

Finally, Libaudioverse contains a subsystem for automatically generating bindings to many programming languages.  Unlike generic solutions, this subsystem "knows" Libaudioverse and takes advantage of semantics of the library.  This means that programming language bindings can never become outdated and rarely need human updating; this system is sufficient that the implementation of new objects automatically reflects itself everywhere.

##Release Targets##

Libaudioverse is approaching a point of being releasable.  The following two release targets are well-defined at the current point of development; if an item needs to be added to these lists, there must be a clear reason for it.  As branches implementing these features get merged, I shall update these lists to reflect what has been done.

- [ ] a

- [x] b

- [ ] c