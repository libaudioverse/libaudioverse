enumerations:
  Lav_ERRORS:
    doc_description: |
      All functions in this library return one of the following enum values, indicating their error condition.
    members:
      Lav_ERROR_NONE: No error occured.
      Lav_ERROR_UNKNOWN: Something went wrong.  This error indicates that we couldn't figure out what.
      Lav_ERROR_TYPE_MISMATCH: Indicates an attempt to manipulate a property through a function that does not work with that property's type.
      Lav_ERROR_INVALID_PROPERTY: An attempt to access a property which does not exist on the specified node.
      Lav_ERROR_NULL_POINTER: You passed a null pointer into Libaudioverse in a context where null pointers are not allowed.
      Lav_ERROR_MEMORY: Libaudioverse triedd to allocate a pointer, but could not.
      Lav_ERROR_INVALID_HANDLE: A value passed in as a handle is not currently a handle which is valid.
      Lav_ERROR_RANGE: A function parameter is not within a valid range.  This could be setting property values outside their range, accessing inputs and outputs that do not exist, or any of a variety of other range error conditions.
      Lav_ERROR_CANNOT_INIT_AUDIO:  The audio subsystem could not be initialized.
      Lav_ERROR_FILE: Represents a miscelaneous file error.
      Lav_ERROR_FILE_NOT_FOUND: Libaudioverse could not find a specified file.
      Lav_ERROR_HRTF_INVALID: An attempt to use an invalid HRTF database.
      Lav_ERROR_CANNOT_CROSS_SIMULATIONS: An attempt was made to relate two objects from different simulations. This could be assigning to buffer properties, connecting nodes, or any other such condition.
      Lav_ERROR_CAUSES_CYCLE: The requested operation would cause a cycle in the graph of nodes that need processing.
      Lav_ERROR_PROPERTY_IS_READ_ONLY: Attempt to set a read-only property.
      Lav_ERROR_OVERLAPPING_AUTOMATORS: An attempt to schedule an automator within the duration of another.
      Lav_ERROR_CANNOT_CONNECT_TO_PROPERTY: Attempt to connect a node to a property which cannot be automated.
      Lav_ERROR_BUFFER_IN_USE: Indicates an attempt to modify a buffer while something is reading its data.
      Lav_ERROR_INTERNAL: If you see this error, it's a bug.
  Lav_PROPERTY_TYPES:
    doc_description: |
      Indicates the type of a property.
    members:
      Lav_PROPERTYTYPE_INT: Property holds a 32-bit integer.
      Lav_PROPERTYTYPE_FLOAT: Property holds a 32-bit floating point value.
      Lav_PROPERTYTYPE_DOUBLE: Property holds a 64-bit double.
      Lav_PROPERTYTYPE_STRING: Property holds a string.
      Lav_PROPERTYTYPE_FLOAT3: Property holds a float3, a vector of 3 floats.
      Lav_PROPERTYTYPE_FLOAT6: Property holds a float6, a vector of 6 floats.
      Lav_PROPERTYTYPE_FLOAT_ARRAY: Property is an array of floats.
      Lav_PROPERTYTYPE_INT_ARRAY: Property is an array of ints.
      Lav_PROPERTYTYPE_BUFFER: Property holds a handle to a buffer.
  Lav_NODE_STATES:
    doc_description: |
      used to indicate the state of a node.
      This is the value of the node's state property and determins how the node is processed.
    members:
      Lav_NODESTATE_PAUSED: This node is paused.
      Lav_NODESTATE_PLAYING: This node advances if other nodes need audio from it.
      Lav_NODESTATE_ALWAYS_PLAYING: This node advances always.
  Lav_LOGGING_LEVELS:
    doc_description: |
      Possible levels for logging.
    members:
      Lav_LOGGING_LEVEL_OFF: No log messages will be generated.
      Lav_LOGGING_LEVEL_CRITICAL: Logs critical messages such as failures to initialize and error conditions.
      Lav_LOGGING_LEVEL_INFO: Logs informative messages.
      Lav_LOGGING_LEVEL_DEBUG: Logs everything possible.
  Lav_PANNING_STRATEGIES:
    doc_description: |
      Indicates a strategy to use for panning.
      This is mostly for the {{"Lav_OBJTYPE_MULTIPANNER_NODE"|node}} and the 3D components of this library.
    members:
      Lav_PANNING_STRATEGY_HRTF: Indicates HRTF panning.
      Lav_PANNING_STRATEGY_STEREO: Indicates stereo panning.
      Lav_PANNING_STRATEGY_SURROUND40: Indicates 4.0 surround sound (quadraphonic) panning.
      Lav_PANNING_STRATEGY_SURROUND51: Indicates 5.1 surround sound panning.
      Lav_PANNING_STRATEGY_SURROUND71: Indicates 7.1 surround sound panning.
  Lav_BIQUAD_TYPES:
    doc_description: |
      Indicates a biquad filter type, used with the {{"Lav_OBJTYPE_BIQUAD_NODE"|node}} and in a few other places.
    members:
      Lav_BIQUAD_TYPE_LOWPASS: Indicates a lowpass filter.
      Lav_BIQUAD_TYPE_HIGHPASS: Indicates a highpass filter.
      Lav_BIQUAD_TYPE_BANDPASS: Indicates a bandpass filter.
      Lav_BIQUAD_TYPE_NOTCH: Indicates a notch filter.
      Lav_BIQUAD_TYPE_ALLPASS: Indicates an allpass filter.
      Lav_BIQUAD_TYPE_PEAKING: Indicates a peaking filter.
      Lav_BIQUAD_TYPE_LOWSHELF: Indicates a lowshelf filter.
      Lav_BIQUAD_TYPE_HIGHSHELF: Indicates a highshelf filter.
      Lav_BIQUAD_TYPE_IDENTITY: This filter does nothing.
  Lav_DISTANCE_MODELS:
    doc_description: |
      used in the 3D components of this library.
      Indicates how sound should become quieter as objects move away from the listener.
    members:
      Lav_DISTANCE_MODEL_LINEAR: Sound falls off as {{"1-(distance/maxDistance)"|codelit}}.
      Lav_DISTANCE_MODEL_EXPONENTIAL: Sounds fall off as {{"1/distance"|codelit}}.
      Lav_DISTANCE_MODEL_INVERSE_SQUARE: Sounds fall off as {{"1/min(distance, maxDistance)^2"|codelit}}.
  Lav_FDN_FILTER_TYPES:
    doc_description: Possible filter types for a feedback delay network's feedback path.
    members:
      Lav_FDN_FILTER_TYPE_DISABLED: Don't insert filters on the feedback path.
      Lav_FDN_FILTER_TYPE_LOWPASS: Insert lowpass filters on the FDN's feedback path.
      Lav_FDN_FILTER_TYPE_HIGHPASS: Insert highpass filters on the FDN's feedback path.
  Lav_CHANNEL_INTERPRETATIONS:
    doc_description: Specifies how to treat inputs to this node for upmixing and downmixing.
    members:
      Lav_CHANNEL_INTERPRETATION_DISCRETE: If channel counts mismatch, don't apply mixing matrices. Either drop or fill with zeros as appropriate.
      Lav_CHANNEL_INTERPRETATION_SPEAKERS: Apply mixing matrices if needed.
  Lav_NOISE_TYPES:
    doc_description: Specifies types of noise.
    members:
      Lav_NOISE_TYPE_WHITE: gaussian white noise.
      Lav_NOISE_TYPE_PINK: Pink noise.  Pink noise falls off at 3 DB per octave.
      Lav_NOISE_TYPE_BROWN: Brown noise.  Brown noise decreases at 6 DB per octave.
  