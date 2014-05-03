Libaudioverse
==============

Libaudioverse is a library for 3D and environmental audio.

Libaudioverse provides high level abstractions for most things: 3D sources and reverb among them.  Simply create a source, load a file, apply whatever effects you want, and start it playing.  Even the type of output is abstracted away.

At its lowest level, Libaudioverse provides a model of digital signal processing, where the signal flow graph can be represented by nodes and edges between them.
Nodes process audio.  They have some number of inputs and some number of outputs.  The edges in the graph are formed by input-output relationships; this graph is directed.
Outputs may be connected to any number of inputs, but an input may be connected to only one output.  How exactly this works is a library internal: you can write your own objects without understanding it, so long as you follow the rules discussed later in this document (todo: define these rules).

In English, you connect outputs to inputs, kind of like plugging in wires.  For example, connecting the sound file reading node to the sound card node will play the sound file.  A sound file input node will have, for example, an output for each channel of the file. Technically, there is no sound card node, but there is an ultimate output node from which audio can be read by whatever wants to play it, and the library does provide an easy way to do so.  As was stated above, you do not need to work at this level.  You nevertheless can if you need to.

##Error Handling##

All functions return an error code.  If the function is a getter, it also returns one or more additional values via pointers.

##Stylistic Convensions##

Everything from this library intended for public use begins with `Lav`, even constants.  The actual stylistic convensions pertain to the rest:

- Constants, macros, and enumeration values are alll uppercase with underscores, for example `Lav_PROPERTYTYPE_INT`.

- Functions are camelcase proceeded by `Lav_`, for example `Lav_getIntProperty`.

- Struct members are not proceedded by `Lav`, as they are part of the type and this is unnecessary; they are lower-case with ubnderscores separating words.

