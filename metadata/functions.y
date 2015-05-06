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
      Failure to do so may cause crashes as Libaudioverse may attempt to make use of resources you passed it as your app shuts down,
      for example callbacks.
      
      Once this function has been called, all pointers and handles from Libaudioverse are invalid.
      Continuing to make use of any resource Libaudioverse has given you after this point will cause crashing.
  Lav_isInitialized:
    category: core
    doc_description: |
      Indicates whether Libaudioverse is initialized.
    params:
      destination: After a call to this function, contains 1 if Libaudioverse is initialized and 0 if it is not.
  Lav_free:
    category: core
    doc_description: |
      Frees pointers that Libaudioverse gives  you.
      There are a limited number of cases wherein Libaudioverse will return a newly allocated pointer.
      Such cases should be clearly documented in this manual.
      In order to free such memory, be sure to use this function rather than the normal system free:
      on some platforms, the Libaudioverse DLL may not be using the same C runtime,
      and the memory passed to you may be allocated from internal caches.
    params:
      ptr: The pointer to free.
  Lav_handleIncRef:
    category: core
    doc_description: |
      Newly allocated Libaudioverse handles have a reference count of 1.
      This function allows incrementing this reference count.
      if you are working in C, this function is not very helpful.
      it is used primarily by the various programming language bindings
      in order to make the garbage collector play nice.
    params:
      handle: The handle whose reference count is to be incremented.
  Lav_handleDecRef:
    category: core
    doc_description: |
      Decrement the reference count of a Libaudioverse handle.
      This function is the equivalent to lav_free for objects.
      Note that this is only a decrement.
      if you call it in the middle of a block or in a variety of other situations, you may see the same handle again via a callback.
      
      It is not possible for Libaudioverse to provide a guaranteed freeing function.
      Such a function would have to block, making it unusable in garbage collected languages.
      Furthermore, in the case of objects like buffers, various internal Libaudioverse properties could conceivably need to hold onto the object.
      Internally, Libaudioverse uses shared pointers to make sure this cannot happen,
      but at the cost of not being able to guarantee instant resource freeing.
  Lav_handleGetAndClearFirstAccess:
    category: core
    doc_description: |
      Checks the handle's first access flag and clears it.
      This is an atomic operation, used by bindings to automatically increment and decrement handle reference counts appropriately.
      Namely, in the case of this function indicating that the first access flag is set, we avoid incrementing the reference count as it is 1 but we have no other external copies of the handle which will be decremented.
    params:
      handle: The handle to check.
      destination: 1 if the first access flag is set, otherwise 0.
  Lav_handleGetRefCount:
    category: core
    doc_description: |
      For debugging.  Allows obtaining the current reference count of the handle.
    params:
      handle: The handle to obtain the reference count of
      destination: After a call to this function, contains the reference count of the handle.
  Lav_handleGetType:
    category: core
    doc_description: |
      Returns the type of the handle.
    params:
      handle: The handle to obtain the type of.
      destination: A Lav_OBJTYPE_* constant corresponding to the handle's type.
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
      You will receive messages via the logging callback for all levels less than or equal to the logging level.
    params:
      level: The new logging level.
  Lav_getLoggingLevel:
    category: core
    doc_description: |
      Get the current logging level
    params:
      destination: Contains the current logging level.
  Lav_setHandleDestroyedCallback:
    category: core
    doc_description: |
      Set the callback to be called when a Libaudioverse handle is permanently destroyed.
      Libaudioverse handles cannot be reused.
      When this callback is called, it is the last time your program can see the specific handle in question,
      and further use of that handle will cause crashes.
      
      This exists primarily for language bindings.
      If there is a case in which your C app must know if a handle is still valid, you may have design issues.
    params:
      cb: The callback to be called when handles are destroyed.
  Lav_deviceGetCount:
    category: core
    doc_description: |
      Get the number of audio devices on the system.
    params:
      destination: Contains the number of audio devices on the system after a call to this function.
  Lav_deviceGetLatency:
    category: core
    doc_description: |
      Gets the latency of the specified audio device.
      Note that this is not likely to be accurate.
      In some cases, it is a guess.
      Querying latency cannot be performed on most platforms.
    params:
      index: The index of the audio device.
      destination: Contains the latency in seconds.
  Lav_deviceGetName:
    category: core
    doc_description: |
      Returns a human-readable name for the specified audio device.
    params:
      index: The index of the audio device.
      destination: Contains a pointer to  a string allocated by Libaudioverse containing the name. Use Lav_free on this string when done with it.
  Lav_deviceGetChannels:
    category: core
    doc_description: |
      Query the maximum number of channels for this device before downmixing occurs.
      You should query the user as to the type of audio they want rather than relying on this function.
      Some operating systems and backends will perform their own downmixing and happily claim 8-channel audio on stereo headphones.
      Furthermore, some hardware and device drivers will do the same.
      It is not possible for Libaudioverse to detect this case.
    params:
      index: The index of the audio device.
      destination: Contains the number of channels the device claims to support.
  lav_createSimulationForDevice:
    category: simulations
    doc_description: |
      Creates a simulation that outputs on the specified audio device and with the specified parameters.
    params:
      index: The audio device to use.
      channels: The number of channels to request from the audio device.
      sr: The sampling rate for all nodes created using this simulation.
      blockSize: The size of each block, in samples.
      mixAhead: The mixahead.  See the explanation in the basics section.
      destination: After a call to this function, contains the handle of the newly created simulation.
  Lav_createReadSimulation:
    category: simulations
    doc_description: |
      Creates a read simulation, a simulation with no associated audio device.
    params:
      sr: The sampling rate of the new simulation.
      blockSize: The block size of the new simulation.
      destination: After a call to this function, contains the handle of the newly created simulation.
  lav_simulationGetBlockSize:
    category: simulations
    doc_description: |
      Query the block size of the specified simulation.
    params:
      simulationHandle: The handle of the simulation.
      destination: The block size of the specified simulation.
  Lav_simulationGetBlock:
    category: simulations
    doc_description: |
      Gets a block of audio from the simulation and advances its time.
      using this with a simulation that is outputting audio to an audio device will not work well.
    params:
      simulationHandle: The handle of the simulation to read a block from.
      channels: The number of channels we want. The simulations' output will be upmixed or downmixed as appropriate.
      mayApplyMixingMatrix: If 0, drop any additional channels in the simulation's output and set any  missing channels in the simulation's output to 0. Otherwise, if we can, apply a mixing matrix.