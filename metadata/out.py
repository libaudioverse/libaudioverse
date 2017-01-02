register_c_category(name = "core", doc_name = "Core Functions", doc_description = r"""The following functions are the most important functions of Libaudioverse.
Writing an app without them is impossible.
""")

c_function(
    name = "Lav_errorGetFile",
    category = "core",
    doc = r"""Get the Libaudioverse cpp file where the most recent error on this thread occured.
The pointer returned is valid until another error occurs on this thread.
This function is mainly for debugging and bindings.
""",
    )

c_function(
    name = "Lav_errorGetLine",
    category = "core",
    doc = r"""Return the source line for the last error that occured on this thread.
This function is mainly for debugging and bindings.
""",
    )

c_function(
    name = "Lav_errorGetMessage",
    category = "core",
    doc = r"""Get the message corresponding to the last error that happened on this thread.
The returned pointer is valid until another error occurs.
The main purpose of this function is debugging and bindings.
""",
    )

c_function(
    name = "Lav_free",
    category = "core",
    doc = r"""Frees pointers that Libaudioverse gives  you.
In order to free pointers from Libaudioverse, be sure to use this function rather than the normal system free.

This function is no-op after shutdown, but should not be used before initialization.
This behavior simplifies writing garbage-collected bindings to Libaudioverse, and should not be relied on in C code.
""",
    param_docs = {
        "ptr": r"""The pointer to free.""",
    })

c_function(
    name = "Lav_getLoggingCallback",
    category = "core",
    doc = r"""Get the logging callback.
""",
    param_docs = {
        "destination": r"""The pointer to the logging callback if set, otherwise NULL.""",
    })

c_function(
    name = "Lav_getLoggingLevel",
    category = "core",
    doc = r"""Get the current logging level
""",
    )

c_function(
    name = "Lav_handleDecRef",
    category = "core",
    doc = r"""Decrement the reference count of a Libaudioverse handle.
This function is the equivalent to Lav_free for objects.
Note that this is only a decrement.
If you call it in the middle of a block or in a variety of other situations, you may see the same handle again via a callback.

This function is no-op after shutdown, but should not be used before initialization.
This behavior simplifies writing garbage-collected bindings to Libaudioverse, and should not be relied on directly by C programs.
""",
    param_docs = {
        "handle": r"""The handle whose reference count we are decrementing.""",
    })

c_function(
    name = "Lav_handleGetAndClearFirstAccess",
    category = "core",
    doc = r"""Checks the handle's first access flag and clears it.
This is an atomic operation, used by bindings to automatically increment and decrement handle reference counts appropriately.
""",
    param_docs = {
        "destination": r"""1 if the first access flag is set, otherwise 0.""","handle": r"""The handle to check.""",
    })

c_function(
    name = "Lav_handleGetRefCount",
    category = "core",
    doc = r"""For debugging.  Allows obtaining the current reference count of the handle.
This function is not guaranteed to be reliable; do not assume that it is correct or change application behavior based off it.
""",
    param_docs = {
        "destination": r"""After a call to this function, contains the reference count of the handle.""","handle": r"""The handle to obtain the reference count of""",
    })

c_function(
    name = "Lav_handleGetType",
    category = "core",
    doc = r"""Returns the type of the handle.
""",
    param_docs = {
        "destination": r"""A {{"Lav_OBJTYPES"|enum}} constant corresponding to the handle's type.""","handle": r"""The handle to obtain the type of.""",
    })

c_function(
    name = "Lav_handleIncRef",
    category = "core",
    doc = r"""Newly allocated Libaudioverse handles have a reference count of 1.
This function allows incrementing this reference count.
If you are working in C, this function is not very helpful.
It is used primarily by the various programming language bindings
in order to make the garbage collector play nice.
""",
    param_docs = {
        "handle": r"""The handle whose reference count is to be incremented.""",
    })

c_function(
    name = "Lav_initialize",
    category = "core",
    doc = r"""This function initializes Libaudioverse.
You must call it before calling any other functions.
""",
    )

c_function(
    name = "Lav_isInitialized",
    category = "core",
    doc = r"""Indicates whether Libaudioverse is initialized.
""",
    )

c_function(
    name = "Lav_setHandleDestroyedCallback",
    category = "core",
    doc = r"""Set the callback to be called when a Libaudioverse handle is permanently destroyed.
Libaudioverse guarantees that handle values will not be recycled.
When this callback is called, it is the last time your program can see the specific handle in question,
and further use of that handle will cause crashes.
""",
    param_docs = {
        "cb": r"""The callback to be called when handles are destroyed.""",
    })

c_function(
    name = "Lav_setLoggingCallback",
    category = "core",
    doc = r"""Configure a callback to receive logging messages.
Note that this function can be called before Libaudioverse initialization.

The callback will receive 3 parameters: level, message, and is_final.
Level is the logging level.
Message is the message to log.
is_final is always 0, save when the message is the last message the logging callback will receive, ever.
Use is_final to determine when to deinitialize your Libaudioverse logging.
""",
    param_docs = {
        "cb": r"""The callback to use for logging.""",
    })

c_function(
    name = "Lav_setLoggingLevel",
    category = "core",
    doc = r"""Set the logging level.
You will receive messages via the logging callback for all levels  greater than the logging level.
""",
    param_docs = {
        "level": r"""The new logging level.""",
    })

c_function(
    name = "Lav_shutdown",
    category = "core",
    doc = r"""Shuts down Libaudioverse.
You must call this function at the end of your application.
Failure to do so may cause crashes.
Once this function has been called, all pointers and handles from Libaudioverse are invalid.
Libaudioverse cannot be safely reinitialized.
""",
    )
register_c_category(name = "servers", doc_name = "Server Functions", doc_description = r"""The following functions relate to and control the server.
""")

c_function(
    name = "Lav_createServer",
    category = "servers",
    doc = r"""Creates a  server.
The new server has no associated audio device.
To make it output, use `Lav_serverSetOutputDevice`.
""",
    param_docs = {
        "sr": r"""The sampling rate of the new server.""","blockSize": r"""The block size of the new server.""",
    })

c_function(
    name = "Lav_serverCallIn",
    category = "servers",
    doc = r"""Schedule a function to run in the future.

This function is either called inside the audio thread or outside the audio thread.
If called inside the audio thread, it must exit quickly and not call the Libaudioverse API.
If called outside the audio thread, it can call the Libaudioverse API and will not block audio.
This is the same as node callbacks.

Time advances for servers if and only if they are processing audio for some purpose; this callback is called in audio time, as it were.
The precision of the callback is limited by the block size.
Smaller block sizes will call callbacks more precisely.
""",
    param_docs = {
        "when": r"""The number of seconds from the current time to call the callback.""","inAudioThread": r"""If nonzero, call the callback in the audio thread.""","cb": r"""The callback to call.""","userdata": r"""An extra parameter that will be passed to the callback.""",
    })

c_function(
    name = "Lav_serverClearOutputDevice",
    category = "servers",
    doc = r"""Clear a server's output device.

This is no-op if no output device has been set.

After a call to this function, it is again safe to use `Lav_serverGetBlock`.
""",
    )

c_function(
    name = "Lav_serverGetBlock",
    category = "servers",
    doc = r"""Gets a block of audio from the server and advances its time.
You must allocate enough space to hold exactly one block of audio: the server's block size times the number of channels requested floating point values.
Note that mixing this function with other output methods invokes undefined behavior.
""",
    param_docs = {
        "channels": r"""The number of channels we want. The servers' output will be upmixed or downmixed as appropriate.""","buffer": r"""The memory to which to write the result.""","serverHandle": r"""The handle of the server to read a block from.""","mayApplyMixingMatrix": r"""If 0, drop any additional channels in the server's output and set any  missing channels in the server's output to 0. Otherwise, if we can, apply a mixing matrix.""",
    })

c_function(
    name = "Lav_serverGetBlockSize",
    category = "servers",
    doc = r"""Query the block size of the specified server.
""",
    )

c_function(
    name = "Lav_serverGetSr",
    category = "servers",
    doc = r"""Query the server's sampling rate.
""",
    )

c_function(
    name = "Lav_serverGetThreads",
    category = "servers",
    doc = r"""Get the number of threads that the server is currently using.
""",
    )

c_function(
    name = "Lav_serverLock",
    category = "servers",
    doc = r"""All operations between a call to this function and a call to {{"Lav_serverUnlock"|function}} will happen together, with no blocks mixed between them.
This is equivalent to assuming that the server is a lock, with  all of the required caution that implies.
No other thread will be able to access this server or objects created from it until {{"Lav_serverUnlock"|function}} is called.
If you do not call {{"Lav_serverUnlock"|function}} in a timely manner, then audio will stop until you do.

Pairs of {{"Lav_serverLock"|function}} and {{"Lav_serverUnlock"|function}} nest safely.
""",
    )

c_function(
    name = "Lav_serverSetBlockCallback",
    category = "servers",
    doc = r"""Set a callback to be called just before every block and in the audio thread.
This callback can and should access the Libaudioverse API:
the point of it is that you can use it to perform tasks where missing even one block would be problematic, i.e. very precise scheduling of events.

This  callback can even block, though this will slow down audio mixing and may cause glitchy audio.
The one thing you should never do in this callback is access anything belonging to another server, as this can cause deadlock.

The callback receives two parameters: the server to which it is associated and the time in server time that corresponds to the beginning of the block about to be mixed.
""",
    param_docs = {
        "callback": r"""The callback to use.""","userdata": r"""An extra parameter that will be passed to the callback.""",
    })

c_function(
    name = "Lav_serverSetOutputDevice",
    category = "servers",
    doc = r"""Set the output device of the server.
Use the literal string "default" for the default audio device.

Note that it is possible to change the output device of a server even after it has been set.

After the output device has been set, calls to `Lav_serverGetBlock` will error.
""",
    param_docs = {
        "device": r"""The output device  the server is to play on.""","channels": r"""The number of channels we wish to output.""",
    })

c_function(
    name = "Lav_serverSetThreads",
    category = "servers",
    doc = r"""Set the number of threads that the server is allowed to use.

The value of the threads parameter may be from 1 to infinity.
When set to 1, processing happens in the thread who calls {{"Lav_serverGetBlock"|function}}.
All other values sleep the thread calling {{"Lav_serverGetBlock"|function}} and perform processing in background threads.
""",
    param_docs = {
        "threads": r"""The number of threads to use for processing.  Must be at least 1.  Typical values include 1 and 1 less than the available cores.""",
    })

c_function(
    name = "Lav_serverUnlock",
    category = "servers",
    doc = r"""Release the internal lock of a server, allowing normal operation to resume.
This is to be used after a call to {{"Lav_serverLock"|function}} and on the same thread as that call; calling it in any other circumstance or on any other thread invokes undefined behavior.

Pairs of {{"Lav_serverLock"|function}} and {{"Lav_serverUnlock"|function}} nest safely.
""",
    )

c_function(
    name = "Lav_serverWriteFile",
    category = "servers",
    doc = r"""Write the server's output to the specified file.

This function advances the server as though {{"Lav_serverGetBlock"|function}} were called multiple times, the number of times determined by {{"duration"|param}}.
As a consequence, it is not possible to use this function while the server is outputting.

The file format is determined from the path.
Recognized extensions include ".wav" and ".ogg", which are guaranteed to work on all systems.
In all cases, reasonable defaults are used for those settings which are specific to the encoder.
""",
    param_docs = {
        "channels": r"""The number of channels in the resulting file.""","path": r"""The path to the audio file to be written.""","duration": r"""Duration of the resulting file, in seconds.""","mayApplyMixingMatrix": r"""1 if applying a mixing matrix should be attempted, 0 if extra channels should be treated as 0 or dropped.  This is the same behavior as with {{"Lav_serverGetBlock"|function}}.""",
    })
register_c_category(name = "devices", doc_name = "Device Enumeration", doc_description = r"""The following functions are for queryng informationa bout audio devices.

Note that channel counts are not reliable, and that you should probably use stereo unless the user explicitly says otherwise.
""")

c_function(
    name = "Lav_deviceGetChannels",
    category = "devices",
    doc = r"""Query the maximum number of channels for this device before downmixing occurs.
You should query the user as to the type of audio they want rather than relying on this function.
Some operating systems and backends will perform their own downmixing and happily claim 8-channel audio on stereo headphones.
Furthermore, some hardware and device drivers will do the same.
It is not possible for Libaudioverse to detect this case.
""",
    param_docs = {
        "index": r"""The index of the audio device.""",
    })

c_function(
    name = "Lav_deviceGetCount",
    category = "devices",
    doc = r"""Get the number of audio devices on the system.
""",
    )

c_function(
    name = "Lav_deviceGetName",
    category = "devices",
    doc = r"""Returns a human-readable name for the specified audio device.

The string that this function outputs is encoded in UTF8.
""",
    param_docs = {
        "index": r"""The index of the audio device.""","destination": r"""Contains a pointer to  a string allocated by Libaudioverse containing the name. Use {{"Lav_free"|function}} on this string when done with it.""",
    })
register_c_category(name = "nodes", doc_name = "Node Functions", doc_description = r"""The following functions work on all Libaudioverse nodes.
""")

c_function(
    name = "Lav_nodeConnect",
    category = "nodes",
    doc = r"""Connect the specified output of the specified node to the specified input of the specified node.

it is an error if this would cause a cycle in the graph of nodes.
""",
    param_docs = {
        "destHandle": r"""The node to whose input we are connecting.""","output": r"""The index of the output to connect.""","nodeHandle": r"""The node whose output we are going to connect.""","input": r"""The input to which to connect.""",
    })

c_function(
    name = "Lav_nodeConnectProperty",
    category = "nodes",
    doc = r"""Connect a node's output to an automatable property.
""",
    param_docs = {
        "slot": r"""The index of the property to which to connect.""","output": r"""The output to connect.""","otherHandle": r"""The node to which we are connecting.""",
    })

c_function(
    name = "Lav_nodeConnectServer",
    category = "nodes",
    doc = r"""Connect the specified output of the specified node to the server's input.
""",
    param_docs = {
        "output": r"""The index of the output to connect.""",
    })

c_function(
    name = "Lav_nodeDisconnect",
    category = "nodes",
    doc = r"""Disconnect the output of the specified node.

If {{"otherHandle"|param}} is 0, disconnect from all inputs.

If {{"otherHandle"|param}} is nonzero, disconnect from the specific node and input combination.
""",
    param_docs = {
        "output": r"""The output to disconnect.""","otherHandle": r"""The node from which to disconnect.""","input": r"""The input of the other node from which to disconnect.""",
    })

c_function(
    name = "Lav_nodeGetArrayPropertyLengthRange",
    category = "nodes",
    doc = r"""Get the allowed range for the length of an array in an array property.
This works on both int and float properties.
""",
    param_docs = {
        "destinationMin": r"""After a call to this function, contains the minimum allowed length.""","destinationMax": r"""After a call to this function, contains the maximum allowed length.""",
    })

c_function(
    name = "Lav_nodeGetBufferProperty",
    category = "nodes",
    doc = r"""Gets the value of a specified buffer property.
""",
    )

c_function(
    name = "Lav_nodeGetDoubleProperty",
    category = "nodes",
    doc = r"""Get the specified double property.
""",
    )

c_function(
    name = "Lav_nodeGetDoublePropertyRange",
    category = "nodes",
    doc = r"""Query the range of a double property.
Note that ranges are meaningless for read-only properties.
""",
    param_docs = {
        "destinationMin": r"""After a call to this function, holds the range's minimum.""","destinationMax": r"""After a call to this function, holds the range's maximum.""",
    })

c_function(
    name = "Lav_nodeGetFloatArrayPropertyLength",
    category = "nodes",
    doc = r"""Get the length of the specified float array property.
""",
    )

c_function(
    name = "Lav_nodeGetFloatProperty",
    category = "nodes",
    doc = r"""Get the specified float property's value.
""",
    )

c_function(
    name = "Lav_nodeGetFloatPropertyRange",
    category = "nodes",
    doc = r"""Get the range of a float property.
Note that ranges are meaningless for read-only properties.
""",
    param_docs = {
        "destinationMin": r"""After a call to this function, holds the range's minimum.""","destinationMax": r"""After a call to this function, holds the range's maximum.""",
    })

c_function(
    name = "Lav_nodeGetInputConnectionCount",
    category = "nodes",
    doc = r"""Get the number of inputs this node has.
""",
    )

c_function(
    name = "Lav_nodeGetIntArrayPropertyLength",
    category = "nodes",
    doc = r"""Get the length of the specified int array property.
""",
    )

c_function(
    name = "Lav_nodeGetIntProperty",
    category = "nodes",
    doc = r"""Get the value of the specified int property.
""",
    )

c_function(
    name = "Lav_nodeGetIntPropertyRange",
    category = "nodes",
    doc = r"""Get the range of an int property.
Note that ranges are meaningless for  read-only properties.
""",
    param_docs = {
        "destinationMin": r"""After a call to this function, holds the range's minimum.""","destinationMax": r"""After a call to this function, holds the range's maximum.""",
    })

c_function(
    name = "Lav_nodeGetOutputConnectionCount",
    category = "nodes",
    doc = r"""Get the number of outputs this node has.
""",
    )

c_function(
    name = "Lav_nodeGetPropertyHasDynamicRange",
    category = "nodes",
    doc = r"""Find out whether or not a property has a dynamic range.
Properties with dynamic ranges change their ranges at specified times, as documented by the documentation for the property of interest.
""",
    param_docs = {
        "destination": r"""After a call to this function, contains 1 if the property has a dynamic range, otherwise 0.""",
    })

c_function(
    name = "Lav_nodeGetPropertyName",
    category = "nodes",
    doc = r"""Get the name of a property.
""",
    param_docs = {
        "destination": r"""After a call to this function, contains a newly allocated string that should be freed with {{"Lav_free"|function}}.  The string is the name of this property.""",
    })

c_function(
    name = "Lav_nodeGetPropertyType",
    category = "nodes",
    doc = r"""Get the type of a property.
""",
    )

c_function(
    name = "Lav_nodeGetServer",
    category = "nodes",
    doc = r"""Get the server that a node belongs to.
""",
    )

c_function(
    name = "Lav_nodeIsolate",
    category = "nodes",
    doc = r"""Equivalent to disconnecting all of the outputs of this node.
After a call to isolate, this node will no longer be affecting audio in any way.
""",
    )

c_function(
    name = "Lav_nodeReadFloatArrayProperty",
    category = "nodes",
    doc = r"""Read the float array property at a specified index.
""",
    param_docs = {
        "index": r"""The index at which to read.""",
    })

c_function(
    name = "Lav_nodeReadIntArrayProperty",
    category = "nodes",
    doc = r"""Read the int array property at a specified index.
""",
    param_docs = {
        "index": r"""The index at which to read.""",
    })

c_function(
    name = "Lav_nodeReplaceFloatArrayProperty",
    category = "nodes",
    doc = r"""Replace the array contained by a float array property with a new array.
Note that, as usual, memory is copied, not shared.
""",
    param_docs = {
        "length": r"""The length of the new array.""","values": r"""The array itself.""",
    })

c_function(
    name = "Lav_nodeReplaceIntArrayProperty",
    category = "nodes",
    doc = r"""Replace the array contained by an int array property with a new array.
Note that, as usual, memory is copied, not shared.
""",
    param_docs = {
        "length": r"""The length of the new array.""","values": r"""The array itself.""",
    })

c_function(
    name = "Lav_nodeReset",
    category = "nodes",
    doc = r"""Reset a node.
What this means depends on the node in question.
Properties are not touched by node resetting.""",
    )

c_function(
    name = "Lav_nodeResetProperty",
    category = "nodes",
    doc = r"""Reset a property to its default.
""",
    )

c_function(
    name = "Lav_nodeSetBufferProperty",
    category = "nodes",
    doc = r"""Set a buffer property.
""",
    param_docs = {
        "value": r"""The buffer to set the property to.  0 means none.""",
    })

c_function(
    name = "Lav_nodeSetDoubleProperty",
    category = "nodes",
    doc = r"""Set the specified double property.
""",
    param_docs = {
        "value": r"""the new value of the property.""",
    })

c_function(
    name = "Lav_nodeSetFloat3Property",
    category = "nodes",
    doc = r"""Set the specified float3 property.
""",
    param_docs = {
        "v1": r"""The first component of the float3.""","v2": r"""The second component of the float3.""","v3": r"""The third component of the float3.""",
    })

c_function(
    name = "Lav_nodeSetFloat6Property",
    category = "nodes",
    doc = r"""Set the specified float6 property.
""",
    param_docs = {
        "v1": r"""The first component of the float6.""","v2": r"""The second component of the float6.""","v3": r"""The third component of the float6.""","v4": r"""The fourth component of the float6.""","v5": r"""The fifth component of the float6.""","v6": r"""The 6th component of the float6.""",
    })

c_function(
    name = "Lav_nodeSetFloatProperty",
    category = "nodes",
    doc = r"""Set the specified float property.
""",
    param_docs = {
        "value": r"""the new value of the property.""",
    })

c_function(
    name = "Lav_nodeSetIntProperty",
    category = "nodes",
    doc = r"""Set an int property.
Note that this function also applies to boolean properties, as these are actually int properties with the range [0, 1].
""",
    param_docs = {
        "value": r"""The new value of the property.""",
    })

c_function(
    name = "Lav_nodeWriteFloatArrayProperty",
    category = "nodes",
    doc = r"""Write a range of values into the specified float array property, without changing its length.
""",
    param_docs = {
        "stop": r"""One past the end of the region to be replaced. Must be no more than the length of the property.""","start": r"""The starting index of the range to replace. Must be less than the length of the property.""","values": r"""the data with which to replace the range. Must have length stop-start.""",
    })

c_function(
    name = "Lav_nodeWriteIntArrayProperty",
    category = "nodes",
    doc = r"""Write a range of values into the specified  int array property, without changing its length.
""",
    param_docs = {
        "stop": r"""One past the end of the region to be replaced. Must be no more than the length of the property.""","start": r"""The starting index of the range to replace. Must be less than the length of the property.""","values": r"""the data with which to replace the range. Must have length stop-start.""",
    })
register_c_category(name = "automators", doc_name = "Automator Functions", doc_description = r"""The following functions manipulate automatable properties.
""")

c_function(
    name = "Lav_automationCancelAutomators",
    category = "automators",
    doc = r"""Cancel all automators that are scheduled to begin running after the specified time.
""",
    param_docs = {
        "time": r"""The time after which to cancel automation.  This is relative to the node.""",
    })

c_function(
    name = "Lav_automationEnvelope",
    category = "automators",
    doc = r"""An automator that performs an envelope.

The specified points are stretched to fit the specified duration.
At the scheduled time of this automator, the envelope will begin being performed, finishing at {{"time+duration"|codelit}}.

As described in the basics section, it is an error to schedule an automator during the range {{"(time, time+duration)"|codelit}}.
""",
    param_docs = {
        "slot": r"""The index of the property to automate.""","duration": r"""The duration of the envelope.""","valuesLength": r"""The length of the values array.""","values": r"""The points of the envelope, sampled every {{"duration/valuesLength"|codelit}} seconds.""","time": r"""The time at which the envelope should begin.""",
    })

c_function(
    name = "Lav_automationLinearRampToValue",
    category = "automators",
    doc = r"""Sets up a linear ramp.

The value of a linear ramp begins at the end of the last automator and linearly increases to the start time of this automator, after which the property holds steady unless more automators are scheduled.
""",
    param_docs = {
        "slot": r"""The slot of the property to automate.""","value": r"""The value we must arrive at by the specified time.""","time": r"""The time at which we must be at the specified value.""",
    })

c_function(
    name = "Lav_automationSet",
    category = "automators",
    doc = r"""An automator that sets the property's value to a specific value at a specific time.
""",
    param_docs = {
        "slot": r"""The slot of the property to automate.""","value": r"""The value to set the property to at the specified time.""","time": r"""The time at which to set the value.""",
    })
register_c_category(name = "buffers", doc_name = "Buffer Functions", doc_description = r"""The following functions manipulate buffers.
""")

c_function(
    name = "Lav_bufferGetDuration",
    category = "buffers",
    doc = r"""Get the duration of the buffer in seconds.
""",
    param_docs = {
        "bufferHandle": r"""The buffer to retrieve the duration for.""",
    })

c_function(
    name = "Lav_bufferGetLengthInSamples",
    category = "buffers",
    doc = r"""Get the length of the specified buffer in samples.

The sample rate of a buffer is the sample rate of the server for which that buffer was created.
This function is primarily useful for estimating ram usage in caching structures.
""",
    param_docs = {
        "bufferHandle": r"""The buffer whose length is to be queried.""",
    })

c_function(
    name = "Lav_bufferGetServer",
    category = "buffers",
    doc = r"""Get the handle of the server used to create this buffer.
""",
    param_docs = {
        "bufferHandle": r"""The handle of the buffer.""",
    })

c_function(
    name = "Lav_bufferLoadFromArray",
    category = "buffers",
    doc = r"""Load data into the specified buffer from the specified array of floating point audio data.
""",
    param_docs = {
        "bufferHandle": r"""The buffer to load data into.""","sr": r"""The sampling rate of the data in the array.""","data": r"""A pointer to the beginning of the array to load from.""","frames": r"""The number of frames of audio data; frames*channels is the length of the array in samples.""","channels": r"""The number of audio channels in the data; frames*channels is the total length of the array in samples.""",
    })

c_function(
    name = "Lav_bufferLoadFromFile",
    category = "buffers",
    doc = r"""Loads data into this buffer from a file. The file will be resampled to the sampling rate of the server. This will happen synchronously.""",
    param_docs = {
        "bufferHandle": r"""The buffer into which to load data.""","path": r"""The path to the file to load data from.""",
    })

c_function(
    name = "Lav_bufferNormalize",
    category = "buffers",
    doc = r"""Normalize the buffer.
This function divides by the sample whose absolute value is greatest.
The effect is to make sounds as loud as possible without clipping or otherwise distorting the sound.
""",
    param_docs = {
        "bufferHandle": r"""The buffer to normalize.""",
    })

c_function(
    name = "Lav_createBuffer",
    category = "buffers",
    doc = r"""Create an empty buffer.
""",
    )
