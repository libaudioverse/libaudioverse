properties:
  Lav_OSCILLATOR_FREQUENCY:
    name: frequency
    type: float
    default: 440.0
    range: [0, INFINITY]
    rate: a
    doc_description: |
      The frequency of the impulse train in HZ.
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
      The phase of the impulse train.
      This is measured in periods, not in radians.
  Lav_BLIT_HARMONICS:
    name: harmonics
    type: int
    default: 0
    range: [0, MAX_INT]
    doc_description: |
      The number of harmonics to include.
      0 requests automatic adjustment.  When 0, the algorithm this node represents will include as many harmonics as it can without aliasing.
  Lav_BLIT_SHOULD_NORMALIZE:
    name: should_normalize
    type: boolean
    default: 1
    doc_description: |
      If false, the produced BLIT will have an integral of 1 over every period.
      If true, the produced blit will be normalized to be between 1 and -1, and suitable for audio.
      The default is true.
inputs: null
outputs:
  - [1, "A bandlimited impulse train."]
doc_name: blit
doc_description: |
  Generates bandlimited impulse trains.  These sound like a buzz, but have important applications in the  alias-free synthesis of analog waveforms.