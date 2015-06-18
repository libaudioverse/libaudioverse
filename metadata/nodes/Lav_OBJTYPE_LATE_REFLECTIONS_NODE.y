properties:
  Lav_LATE_REFLECTIONS_T60:
    name: t60
    type: float
    default: 0.3
    range: [0.05, INFINITY]
    doc_description: |
      The time it takes for the late reflections to decay to {{"-60 DB"|codelit}}.
inputs: described
outputs: described
doc_name: late reflections
doc_description: |
  A late reflections generator.
  
  This node generates the late reflections portion of the Libaudioverse reverb using a 16-line FDN algorithm.
  
  This node always has 16 inputs and 16 outputs, as it is primarily used internally.
  if you wish to use this node in your own code, perhaps the simplest is to feed the same signal to all channels.

  The inputs represent the inputs to the internal FDN, and the outputs represent the outputs.