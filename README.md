Libaudioverse
==============

The new Camlorn_audio, implementing all the stuff the old one can't by outright abandoning OpenAL.  Planned features include sample-perfect callbacks on some objects, analyzers, microphone, reverb, and filters with an arbetrary frequency spectrum, but not necessarily in that order.

Libaudioverse provides a low-level model of digital signal processing, where the signal flow graph can be represented by nodes and edges between them.
Nodes process audio.  They have some number of inputs and some number of outputs.  The edges in the graph are formed by input-output relationships; this graph is directed.
Outputs may be connected to any number of inputs, but an input may be connected to only one output.  How exactly this works is a library internal: you can write your own objects without understanding it, so long as you follow the rules discussed later in this document (todo: define these rules).

In English, you connect outputs to inputs, kind of like plugging in wires.  For example, connecting the sound file reading node to the sound card node will play the sound file.  A sound file input node will have, for example, an output for each channel of the file. Technically, there is no sound card node, but there is an ultimate output node from which audio can be read by whatever wants to play it, and the library does provide an easy way to do so.
The specific technique is to call the process method on the output node in a thread, and read from its outputs.

In order to support cyclic graphs, a 1-sample delay is introduced at every input-output connection.  There is literally no other way to do this sensibly, and it is how things actually work in the real world (electrons do take time to travel down a wire, we just can't notice it).