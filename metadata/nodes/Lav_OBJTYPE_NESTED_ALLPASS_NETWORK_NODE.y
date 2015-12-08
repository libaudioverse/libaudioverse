extra_functions:
  Lav_nestedAllpassNetworkNodeBeginNesting:
    doc_description: |
      Append a first-order direct form II allpass to the current network, and move our focus inside it.
      All appended filters will be appended to the allpass's internal delay line until such time as you call {{"Lav_nestedAllpassNetworkNodeEndNesting"|extra_function}}.
    params:
      delay: The delay of the newly created allpass in samples.  This must be at least 1.
      coefficient: The coefficient of the new allpass.
  Lav_nestedAllpassNetworkNodeEndNesting:
    doc_description: |
      End the current nesting.
      This moves the focus to the end of the allpass filter which we are currently nesting inside of.
      Behavior is undefined if this function is called without an enclosing allpass filter, but will probably crash the application.
  Lav_nestedAllpassNetworkNodeAppendAllpass:
    doc_description: |
      Append an ordinary first-order allpass at the current nesting level.
    params:
      delay: The delay of the allpass in samples.  Must be at least 1.
      coefficient: The coefficient of the allpass.
  Lav_nestedAllpassNetworkNodeAppendOnePole:
    doc_description: |
      Appenda  one-pole filter.
    params:
      frequency: The {{"-3 DB"|codelit}} frequency.
      isHighpass: 0 for lowpass, 1 for highpass.
  Lav_nestedAllpassNetworkNodeAppendBiquad:
    doc_description: |
      Append a biquad filter.
      This is the same as the {{"Lav_OBJTYPE_BIQUAD_NODE"|node}}.
    params:
      type: A value from the {{"Lav_BIQUAD_TYPES"|enum}} enum.
      frequency: The frequency of interest.  The meaning is determined from the filter type.
      dbGain: Used for some filter types, namely highshelf, lowshelf, and notch.  values are in DB.
      q: The q of the filter.
  Lav_nestedAllpassNetworkNodeAppendReader:
    doc_description: |
      The output will include audio from wherever this is appended.
      You need to call this function at least once, or your configured filter will be silent.
    params:
      mul: The volume of the reader.
      mul: The volume of the reader.
  Lav_nestedAllpassNetworkNodeCompile:
    doc_description: |
      Compile the current set of commands, replace the currently running filter with the new one, and clear the current set of commands.
inputs:
  - [constructor, "The signal to filter."]
outputs:
  - [constructor, "The filtered signal."]
doc_name: nested allpass network
doc_description: |
  
  This node is deprecated.
  A more capable node along the same lines is planned, at which point this one will disappear.
  
  This node implements nested first-order allpass filters.
  
  In order to specify how this nesting works, one must call the various control functions.
  This node's operation is somewhat analogous to old-style OpenGL programming: there is an implicit context that is manipulated via calling functions, as opposed to a set of properties to be set.
  
  Filters may be appended in series.
  At any time, however, it is possible to append a nested allpass filter.
  When a nested allpass filter is appended, any new filters are appended to the end of the nested allpass's delay line; call the function {{"Lav_nestedAllpassNodeEndNesting"|extra_function}} to end nesting and return to the previous level.
  Multiple levels of nesting are supported.
  
  In order to read this node's output, be sure to call {{"Lav_nestedAllpassNetworkNodeAppendReader"|extra_function}}; any audio that heads through this special no-op filter will be read and placed in this node's output buffers.
  
  In order to finalize the construction of a network, call {{"Lav_nestedAllpassNetworkNodeCompile"|extra_function}}.
  This function takes all previously issued commands, wraps them up, and uses them to replace the current network.
  After a call to this function, all subsequent commands are building a new network whose changes will not  be heard until this function is called.
  
  The default configuration of this node is silence.  To return to this configuration at any time, call {{"Lav_nestedAllpassNetworkNodeCompile"|extra_function}} without issuing any commands first.
  
  Note that this node is extremely slow as compared to other Libaudioverse nodes.
  The primary use of this node is for experimentation purposes and the development of faster Libaudioverse nodes.