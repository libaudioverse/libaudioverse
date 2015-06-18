properties:
  Lav_LATE_REFLECTIONS_T60:
    name: t60
    type: float
    default: 0.3
    range: [0.05, INFINITY]
    doc_description: |
      The time it takes for the late reflections to decay by {{"-60 DB"|codelit}}.
      
      Higher values are indicative of larger spaces.
  Lav_LATE_REFLECTIONS_DENSITY:
    name: density
    default: 500
    type: float
    range: [100.0, 1000.0]
    doc_description: |
      The average number of reflections per second.
      
      Higher values are generally indicative of more complex and reflective surfaces, placed closer together.
      As an example, a carpetted cathedral would have a lower value than a small, concrete room.
inputs: described
outputs: described
doc_name: late reflections
doc_description: |
  A late reflections generator.
  
  This node generates the late reflections portion of the Libaudioverse reverb using a 16-line FDN algorithm.
  
  This node always has 16 inputs and 16 outputs, as it is primarily used internally.
  if you wish to use this node in your own code, perhaps the simplest is to feed the same signal to all channels.

  The inputs represent the inputs to the internal FDN, and the outputs represent the outputs.