functions:
  Lav_initialize:
    category: core
    doc_description: |
      This function initializes Libaudioverse.
      You must call it before calling any other functions.
  Lav_shutdown:
    category: core
    doc_description: |
      Shuts down Libaudioverse.
      You must call this function at the end of your application.
      Failure to do so may cause crashes.
      Once this function has been called, all pointers and handles from Libaudioverse are invalid.
      Libaudioverse cannot be safely reinitialized.
  Lav_isInitialized:
    category: core
    doc_description: |
      Indicates whether Libaudioverse is initialized.
  Lav_errorGetMessage:
    category: core
    doc_description: |
      Get the message corresponding to the last error that happened on this thread.
      The returned pointer is valid until another error occurs.
      The main purpose of this function is debugging and bindings.
  Lav_errorGetFile:
    category: core
    doc_description: |
      Get the Libaudioverse cpp file where the most recent error on this thread occured.
      The pointer returned is valid until another error occurs on this thread.
      This function is mainly for debugging and bindings.
  Lav_errorGetLine:
    category: core
    doc_description: |
      Return the source line for the last error that occured on this thread.
      This function is mainly for debugging and bindings.
  Lav_free:
    category: core
    doc_description: |
      Frees pointers that Libaudioverse gives  you.
      In order to free pointers from Libaudioverse, be sure to use this function rather than the normal system free.
      
      This function is no-op after shutdown, but should not be used before initialization.
      This behavior simplifies writing garbage-collected bindings to Libaudioverse, and should not be relied on in C code.
    params:
      ptr: The pointer to free.
  Lav_handleIncRef:
    category: core
    doc_description: |
      Newly allocated Libaudioverse handles have a reference count of 1.
      This function allows incrementing this reference count.
      If you are working in C, this function is not very helpful.
      It is used primarily by the various programming language bindings
      in order to make the garbage collector play nice.
    params:
      handle: The handle whose reference count is to be incremented.
  Lav_handleDecRef:
    category: core
    doc_description: |
      Decrement the reference count of a Libaudioverse handle.
      This function is the equivalent to Lav_free for objects.
      Note that this is only a decrement.
      If you call it in the middle of a block or in a variety of other situations, you may see the same handle again via a callback.
      
      This function is no-op after shutdown, but should not be used before initialization.
      This behavior simplifies writing garbage-collected bindings to Libaudioverse, and should not be relied on directly by C programs.
    params:
      handle: The handle whose reference count we are decrementing.
  Lav_handleGetAndClearFirstAccess:
    category: core
    doc_description: |
      Checks the handle's first access flag and clears it.
      This is an atomic operation, used by bindings to automatically increment and decrement handle reference counts appropriately.
    params:
      handle: The handle to check.
      destination: 1 if the first access flag is set, otherwise 0.
  Lav_handleGetRefCount:
    category: core
    doc_description: |
      For debugging.  Allows obtaining the current reference count of the handle.
      This function is not guaranteed to be reliable; do not assume that it is correct or change application behavior based off it.
    params:
      handle: The handle to obtain the reference count of
      destination: After a call to this function, contains the reference count of the handle.
  Lav_handleGetType:
    category: core
    doc_description: |
      Returns the type of the handle.
    params:
      handle: The handle to obtain the type of.
      destination: A {{"Lav_OBJTYPES"|enum}} constant corresponding to the handle's type.
  Lav_setLoggingCallback:
    category: core
    doc_description: |
      Configure a callback to receive logging messages.
      Note that this function can be called before Libaudioverse initialization.
      
      The callback will receive 3 parameters: level, message, and is_final.
      Level is the logging level.
      Message is the message to log.
      is_final is always 0, save when the message is the last message the logging callback will receive, ever.
      Use is_final to determine when to deinitialize your Libaudioverse logging.
    params:
      cb: The callback to use for logging.
  Lav_getLoggingCallback:
    category: core
    doc_description: |
      Get the logging callback.
    params:
      destination: The pointer to the logging callback if set, otherwise NULL.
  Lav_setLoggingLevel:
    category: core
    doc_description: |
      Set the logging level.
      You will receive messages via the logging callback for all levels  greater than the logging level.
    params:
      level: The new logging level.
  Lav_getLoggingLevel:
    category: core
    doc_description: |
      Get the current logging level
  Lav_setHandleDestroyedCallback:
    category: core
    doc_description: |
      Set the callback to be called when a Libaudioverse handle is permanently destroyed.
      Libaudioverse guarantees that handle values will not be recycled.
      When this callback is called, it is the last time your program can see the specific handle in question,
      and further use of that handle will cause crashes.
    params:
      cb: The callback to be called when handles are destroyed.
  Lav_deviceGetCount:
    category: devices
    doc_description: |
      Get the number of audio devices on the system.
  Lav_deviceGetName:
    category: devices
    doc_description: |
      Returns a human-readable name for the specified audio device.
      
      The string that this function outputs is encoded in UTF8.
    params:
      index: The index of the audio device.
      destination: Contains a pointer to  a string allocated by Libaudioverse containing the name. Use {{"Lav_free"|function}} on this string when done with it.
  Lav_deviceGetChannels:
    category: devices
    doc_description: |
      Query the maximum number of channels for this device before downmixing occurs.
      You should query the user as to the type of audio they want rather than relying on this function.
      Some operating systems and backends will perform their own downmixing and happily claim 8-channel audio on stereo headphones.
      Furthermore, some hardware and device drivers will do the same.
      It is not possible for Libaudioverse to detect this case.
    params:
      index: The index of the audio device.
  Lav_createSimulation:
    category: simulations
    doc_description: |
      Creates a  simulation.
      The new simulation has no associated audio device.
      To make it output, use `Lav_simulationSetOutputDevice`.
    params:
      sr: The sampling rate of the new simulation.
      blockSize: The block size of the new simulation.
  Lav_simulationGetBlockSize:
    category: simulations
    doc_description: |
      Query the block size of the specified simulation.
  Lav_simulationGetBlock:
    category: simulations
    doc_description: |
      Gets a block of audio from the simulation and advances its time.
      You must allocate enough space to hold exactly one block of audio: the simulation's block size times the number of channels requested floating point values.
      Note that mixing this function with other output methods invokes undefined behavior.
    params:
      simulationHandle: The handle of the simulation to read a block from.
      channels: The number of channels we want. The simulations' output will be upmixed or downmixed as appropriate.
      mayApplyMixingMatrix: If 0, drop any additional channels in the simulation's output and set any  missing channels in the simulation's output to 0. Otherwise, if we can, apply a mixing matrix.
      buffer: The memory to which to write the result.
  Lav_simulationGetSr:
    category: simulations
    doc_description: |
      Query the simulation's sampling rate.
  Lav_simulationSetOutputDevice:
    category: simulations
    doc_description: |
      Set the output device of the simulation.
      -1 is the system default.  0 and above are specific audio devices, matching the indices returned by the enumeration functions.
      
      Note that it is possible to change the output device of a simulation even after it has been set.
      
      After the output device has been set, calls to `Lav_simulationGetBlock` will error.
      
      The parameters {{"minLatency"|param}}, {{"startLatency"|param}}, and {{"maxLatency"|param}} are hints as to the latency you wish to have.
      The audio backends attempt to optimize themselves according to the CPU load of your application and other factors.
      These parameters control this optimization.
      If your application can suddenly and frequently cause CPU usage to spike, set {{"minLatency"|param}} to something greater than 0.
      The {{"startLatency"|param}} parameter is the latency at which the audio backend will start audio.
      This should usually be somewhat high, as applications often begin by creating a large number of objects.
      The audio backends will reduce latency as rapidly as is safe.
    params:
      index: The output device  the simulation is to play on.
      channels: The number of channels we wish to output.
      minLatency: The minimum latency your application can tolerate. 0.0 is a good value in most cases.
      startLatency: A hint as to the latency the audio backend should start with. 0.05 is a suggested default.
      maxLatency: The maximum latency your application can tolerate. 0.1 or even 0.2 is a suggested default.
  Lav_simulationClearOutputDevice:
    category: simulations
    doc_description: |
      Clear a simulation's output device.
      
      This is no-op if no output device has been set.
      
      After a call to this function, it is again safe to use `Lav_simulationGetBlock`.
  Lav_simulationLock:
    category: simulations
    doc_description: |
      All operations between a call to this function and a call to {{"Lav_simulationUnlock"|function}} will happen together, with no blocks mixed between them.
      This is equivalent to assuming that the simulation is a lock, with  all of the required caution that implies.
      No other thread will be able to access this simulation or objects created from it until {{"Lav_simulationUnlock"|function}} is called.
      If you do not call {{"Lav_simulationUnlock"|function}} in a timely manner, then audio will stop until you do.
      
      Pairs of {{"Lav_simulationLock"|function}} and {{"Lav_simulationUnlock"|function}} nest safely.
  Lav_simulationUnlock:
    category: simulations
    doc_description: |
      Release the internal lock of a simulation, allowing normal operation to resume.
      This is to be used after a call to {{"Lav_simulationLock"|function}} and on the same thread as that call; calling it in any other circumstance or on any other thread invokes undefined behavior.
      
      Pairs of {{"Lav_simulationLock"|function}} and {{"Lav_simulationUnlock"|function}} nest safely.
  Lav_simulationSetBlockCallback:
    category: simulations
    doc_description: |
      Set a callback to be called just before every block and in the audio thread.
      This callback can and should access the Libaudioverse API:
      the point of it is that you can use it to perform tasks where missing even one block would be problematic, i.e. very precise scheduling of events.
      
      This  callback can even block, though this will slow down audio mixing and may cause glitchy audio.
      The one thing you should never do in this callback is access anything belonging to another simulation, as this can cause deadlock.
      
      The callback receives two parameters: the simulation to which it is associated and the time in simulation time that corresponds to the beginning of the block about to be mixed.
    params:
      callback: The callback to use.
      userdata: An extra parameter that will be passed to the callback.
  Lav_simulationWriteFile:
    category: simulations
    doc_description: |
      Write the simulation's output to the specified file.
      
      This function advances the simulation as though {{"Lav_simulationGetBlock"|function}} were called multiple times, the number of times determined by {{"duration"|param}}.
      As a consequence, it is not possible to use this function while the simulation is outputting.
      
      The file format is determined from the path.
      Recognized extensions include ".wav" and ".ogg", which are guaranteed to work on all systems.
      In all cases, reasonable defaults are used for those settings which are specific to the encoder.
    params:
      path: The path to the audio file to be written.
      channels: The number of channels in the resulting file.
      duration: Duration of the resulting file, in seconds.
      mayApplyMixingMatrix: 1 if applying a mixing matrix should be attempted, 0 if extra channels should be treated as 0 or dropped.  This is the same behavior as with {{"Lav_simulationGetBlock"|function}}.
  Lav_simulationSetThreads:
    category: simulations
    doc_description: |
      Set the number of threads that the simulation is allowed to use.
      
      The value of the threads parameter may be from 1 to infinity.
      When set to 1, processing happens in the thread who calls {{"Lav_simulationGetBlock"|function}}.
      All other values sleep the thread calling {{"Lav_simulationGetBlock"|function}} and perform processing in background threads.
    params:
      threads: The number of threads to use for processing.  Must be at least 1.  Typical values include 1 and 1 less than the available cores.
  Lav_simulationGetThreads:
    category: simulations
    doc_description: |
      Get the number of threads that the simulation is currently using.
  Lav_createBuffer:
    category: buffers
    doc_description: |
      Create an empty buffer.
  Lav_bufferGetSimulation:
    category: buffers
    doc_description: |
      Get the handle of the simulation used to create this buffer.
    params:
      bufferHandle: The handle of the buffer.
  Lav_bufferLoadFromFile:
    category: buffers
    doc_description:
      Loads data into this buffer from a file.
      The file will be resampled to the sampling rate of the simulation.
      This will happen synchronously.
    params:
      bufferHandle: The buffer into which to load data.
      path: The path to the file to load data from.
  Lav_bufferLoadFromArray:
    category: buffers
    doc_description: |
      Load data into the specified buffer from the specified array of floating point audio data.
    params:
      bufferHandle: The buffer to load data into.
      sr: The sampling rate of the data in the array.
      channels: The number of audio channels in the data; frames*channels is the total length of the array in samples.
      frames: The number of frames of audio data; frames*channels is the length of the array in samples.
      data: A pointer to the beginning of the array to load from.
  Lav_bufferNormalize:
    category: buffers
    doc_description: |
      Normalize the buffer.
      This function divides by the sample whose absolute value is greatest.
      The effect is to make sounds as loud as possible without clipping or otherwise distorting the sound.
    params:
      bufferHandle: The buffer to normalize.
  Lav_bufferGetDuration:
    category: buffers
    doc_description: |
      Get the duration of the buffer in seconds.
    params:
      bufferHandle: The buffer to retrieve the duration for.
  Lav_bufferGetLengthInSamples:
    category: buffers
    doc_description: |
      Get the length of the specified buffer in samples.
      
      The sample rate of a buffer is the sample rate of the simulation for which that buffer was created.
      This function is primarily useful for estimating ram usage in caching structures.
    params:
      bufferHandle: The buffer whose length is to be queried.
  Lav_nodeGetSimulation:
    category: nodes
    doc_description: |
      Get the simulation that a node belongs to.
  Lav_nodeConnect:
    category: nodes
    doc_description: |
      Connect the specified output of the specified node to the specified input of the specified node.
      
      it is an error if this would cause a cycle in the graph of nodes.
    params:
      nodeHandle: The node whose output we are going to connect.
      output: The index of the output to connect.
      destHandle: The node to whose input we are connecting.
      input: The input to which to connect.
  Lav_nodeConnectSimulation:
    category: nodes
    doc_description: |
      Connect the specified output of the specified node to the simulation's input.
    params:
      output: The index of the output to connect.
  Lav_nodeConnectProperty:
    category: nodes
    doc_description: |
      Connect a node's output to an automatable property.
    params:
      output: The output to connect.
      otherHandle: The node to which we are connecting.
      slot: The index of the property to which to connect.
  Lav_nodeDisconnect:
    category: nodes
    doc_description: |
      Disconnect the output of the specified node.
      
      If {{"otherHandle"|param}} is 0, disconnect from all inputs.
      
      If {{"otherHandle"|param}} is nonzero, disconnect from the specific node and input combination.
    params:
      output: The output to disconnect.
      otherHandle: The node from which to disconnect.
      input: The input of the other node from which to disconnect.
  Lav_nodeIsolate:
    category: nodes
    doc_description: |
      Equivalent to disconnecting all of the outputs of this node.
      After a call to isolate, this node will no longer be affecting audio in any way.
  Lav_nodeGetInputConnectionCount:
    category: nodes
    doc_description: |
      Get the number of inputs this node has.
  Lav_nodeGetOutputConnectionCount:
    category: nodes
    doc_description: |
      Get the number of outputs this node has.
  Lav_nodeResetProperty:
    category: nodes
    doc_description: |
      Reset a property to its default.
  Lav_nodeSetIntProperty:
    category: nodes
    doc_description: |
      Set an int property.
      Note that this function also applies to boolean properties, as these are actually int properties with the range [0, 1].
    params:
      value: The new value of the property.
  Lav_nodeSetFloatProperty:
    category: nodes
    doc_description: |
      Set the specified float property.
    params:
      value: the new value of the property.
  Lav_nodeSetDoubleProperty:
    category: nodes
    doc_description: |
      Set the specified double property.
    params:
      value: the new value of the property.
  Lav_nodeSetStringProperty:
    category: nodes
    doc_description: |
      Set the specified string property.
    params:
      value: the new value of the property.  Note that the specified string is copied and the memory may be freed.
  Lav_nodeSetFloat3Property:
    category: nodes
    doc_description: |
      Set the specified float3 property.
    params:
      v1: The first component of the float3.
      v2: The second component of the float3.
      v3: The third component of the float3.
  Lav_nodeSetFloat6Property:
    category: nodes
    doc_description: |
      Set the specified float6 property.
    params:
      v1: The first component of the float6.
      v2: The second component of the float6.
      v3: The third component of the float6.
      v4: The fourth component of the float6.
      v5: The fifth component of the float6.
      v6: The 6th component of the float6.
  Lav_nodeGetIntProperty:
    category: nodes
    doc_description: |
      Get the value of the specified int property.
  Lav_nodeGetFloatProperty:
    category: nodes
    doc_description: |
      Get the specified float property's value.
  Lav_nodeGetDoubleProperty:
    category: nodes
    doc_description: |
      Get the specified double property.
  Lav_nodeGetStringProperty:
    category: nodes
    doc_description: |
      Get the specified string property.
    params:
      destination: After a call to this function, contains a pointer to a newly allocated string that is a copy of the value of the property.  Free this string with {{"Lav_free"|function}}.
  Lav_nodeGetIntPropertyRange:
    category: nodes
    doc_description: |
      Get the range of an int property.
      Note that ranges are meaningless for  read-only properties.
    params:
      destinationMin: After a call to this function, holds the range's minimum.
      destinationMax: After a call to this function, holds the range's maximum.
  Lav_nodeGetFloatPropertyRange:
    category: nodes
    doc_description: |
      Get the range of a float property.
      Note that ranges are meaningless for read-only properties.
    params:
      destinationMin: After a call to this function, holds the range's minimum.
      destinationMax: After a call to this function, holds the range's maximum.
  Lav_nodeGetDoublePropertyRange:
    category: nodes
    doc_description: |
      Query the range of a double property.
      Note that ranges are meaningless for read-only properties.
    params:
      destinationMin: After a call to this function, holds the range's minimum.
      destinationMax: After a call to this function, holds the range's maximum.
  Lav_nodeGetPropertyName:
    category: nodes
    doc_description: |
      Get the name of a property.
    params:
      destination: After a call to this function, contains a newly allocated string that should be freed with {{"Lav_free"|function}}.  The string is the name of this property.
  Lav_nodeGetPropertyType:
    category: nodes
    doc_description: |
      Get the type of a property.
  Lav_nodeGetPropertyHasDynamicRange:
    category: nodes
    doc_description: |
      Find out whether or not a property has a dynamic range.
      Properties with dynamic ranges change their ranges at specified times, as documented by the documentation for the property of interest.
    params:
      destination: After a call to this function, contains 1 if the property has a dynamic range, otherwise 0.
  Lav_nodeReplaceFloatArrayProperty:
    category: nodes
    doc_description: |
      Replace the array contained by a float array property with a new array.
      Note that, as usual, memory is copied, not shared.
    params:
      length: The length of the new array.
      values: The array itself.
  Lav_nodeReadFloatArrayProperty:
    category: nodes
    doc_description: |
      Read the float array property at a specified index.
    params:
      index: The index at which to read.
  Lav_nodeWriteFloatArrayProperty:
    category: nodes
    doc_description: |
      Write a range of values into the specified float array property, without changing its length.
    params:
      start: The starting index of the range to replace. Must be less than the length of the property.
      stop: One past the end of the region to be replaced. Must be no more than the length of the property.
      values: the data with which to replace the range. Must have length stop-start.
  Lav_nodeGetFloatArrayPropertyLength:
    category: nodes
    doc_description: |
      Get the length of the specified float array property.
  Lav_nodeReplaceIntArrayProperty:
    category: nodes
    doc_description: |
      Replace the array contained by an int array property with a new array.
      Note that, as usual, memory is copied, not shared.
    params:
      length: The length of the new array.
      values: The array itself.
  Lav_nodeReadIntArrayProperty:
    category: nodes
    doc_description: |
      Read the int array property at a specified index.
    params:
      index: The index at which to read.
  Lav_nodeWriteIntArrayProperty:
    category: nodes
    doc_description: |
      Write a range of values into the specified  int array property, without changing its length.
    params:
      start: The starting index of the range to replace. Must be less than the length of the property.
      stop: One past the end of the region to be replaced. Must be no more than the length of the property.
      values: the data with which to replace the range. Must have length stop-start.
  Lav_nodeGetIntArrayPropertyLength:
    category: nodes
    doc_description: |
      Get the length of the specified int array property.
  Lav_nodeGetArrayPropertyLengthRange:
    category: nodes
    doc_description: |
      Get the allowed range for the length of an array in an array property.
      This works on both int and float properties.
    params:
      destinationMin: After a call to this function, contains the minimum allowed length.
      destinationMax: After a call to this function, contains the maximum allowed length.
  Lav_nodeSetBufferProperty:
    category: nodes
    doc_description: |
      Set a buffer property.
    params:
      value: The buffer to set the property to.  0 means none.
  Lav_nodeGetBufferProperty:
    category: nodes
    doc_description: |
      Gets the value of a specified buffer property.
  Lav_automationCancelAutomators:
    category: automators
    doc_description: |
      Cancel all automators that are scheduled to begin running after the specified time.
    params:
      time: The time after which to cancel automation.  This is relative to the node.
  Lav_automationLinearRampToValue:
    category: automators
    doc_description: |
      Sets up a linear ramp.
      
      The value of a linear ramp begins at the end of the last automator and linearly increases to the start time of this automator, after which the property holds steady unless more automators are scheduled.
    params:
      slot: The slot of the property to automate.
      time: The time at which we must be at the specified value.
      value: The value we must arrive at by the specified time.
  Lav_automationSet:
    category: automators
    doc_description: |
      An automator that sets the property's value to a specific value at a specific time.
    params:
      slot: The slot of the property to automate.
      time: The time at which to set the value.
      value: The value to set the property to at the specified time.
  Lav_automationEnvelope:
    category: automators
    doc_description: |
      An automator that performs an envelope.
      
      The specified points are stretched to fit the specified duration.
      At the scheduled time of this automator, the envelope will begin being performed, finishing at {{"time+duration"|codelit}}.
      
      As described in the basics section, it is an error to schedule an automator during the range {{"(time, time+duration)"|codelit}}.
    params:
      slot: The index of the property to automate.
      time: The time at which the envelope should begin.
      duration: The duration of the envelope.
      valuesLength: The length of the values array.
      values: The points of the envelope, sampled every {{"duration/valuesLength"|codelit}} seconds.
  Lav_nodeReset:
    category: nodes
    doc_description: |
      Reset a node.
      What this means depends on the node in question.
      Properties are not touched by node resetting.