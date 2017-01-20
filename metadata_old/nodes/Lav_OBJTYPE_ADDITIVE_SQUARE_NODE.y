properties:
  Lav_OSCILLATOR_FREQUENCY:
    name: frequency
    type: float
    default: 440.0
    range: [0, INFINITY]
    doc_description: |
      The frequency of the square wave, in hertz.
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
      The phase of the square wave.
      This is measured in periods, not in radians.
  Lav_SQUARE_HARMONICS:
    name: harmonics
    type: int
    range: [0, MAX_INT]
    default: 0
    doc_description: |
      The number of harmonics.
      0 requests automatic adjustment.
      Use a nonzero value if you intend to sweep the square wave.
      
      While this property has no max value, any combination of frequency and harmonics that leads to aliasing will alias.
      To avoid this, make sure that {{"2*frequency*(2*harmonics-1)"|codelit}} never goes over half your chosen sampling rate.
inputs: null
outputs:
  - [1, "A square wave."]
doc_name: additive square
doc_description: |
  The most accurate, least featureful, and slowest square oscillator.
  
  This oscillator uses additive synthesis to produce square waves.
  The efficiency therefore depends on the frequency.
  Sweeping this oscillator will perform poorly if you do not set the harmonics to a nonzero value.
  
  This oscillator is slightly under the range -1 to 1.
  For this reason, it is probably not suitable as a control signal.
  This is not fixable using additive synthesis and is a frequency dependent effect.