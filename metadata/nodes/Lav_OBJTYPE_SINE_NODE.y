properties:
  Lav_OSCILLATOR_FREQUENCY:
    name: frequency
    type: float
    default: 440.0
    range: [0, INFINITY]
    rate: a
    doc_description: |
      The frequency of the sine wave in HZ.
  Lav_OSCILLATOR_FREQUENCY_MULTIPLIER:
    name: frequency_multiplier
    type: float
    default: 1.0
    range: [-INFINITY, INFINITY]
    rate: a
    doc_description: |
      An additional multiplicative factor applied to the frequency of the oscillator.
      
      This is useful for creating instruments, as the notes of the standard musical scale fall on frequency multiples of a reference pitch, rather than a linear increase.
  Lav_OSCILLATOR_PHASE:
    name: phase
    range: [0.0, 1.0]
    type: float
    default: 0.0
    doc_description: |
      The phase of the sine node.
      This is measured in periods, not in radians.
inputs: null
outputs:
  - [1, "A sine wave."]
doc_name: sine
doc_description: |
  A simple sine oscillator.
