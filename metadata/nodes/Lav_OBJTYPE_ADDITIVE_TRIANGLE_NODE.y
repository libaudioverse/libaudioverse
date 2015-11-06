properties:
  Lav_TRIANGLE_FREQUENCY:
    name: frequency
    type: float
    default: 440.0
    range: [0, INFINITY]
    doc_description: |
      The frequency of the triangle wave, in hertz.
  Lav_TRIANGLE_PHASE:
    name: phase
    range: [0.0, 1.0]
    type: float
    default: 0.0
    doc_description: |
      The phase of the triangle wave.
      This is measured in periods, not in radians.
  Lav_TRIANGLE_HARMONICS:
    name: harmonics
    type: int
    range: [0, MAX_INT]
    default: 0
    doc_description: |
      The number of harmonics.
      0 requests automatic adjustment.
      use a nonzero value if you intend to sweep the triangle wave.
      
      While this property has no max value, any combination of frequency and harmonics that leads to aliasing will alias.
      To avoid this, make sure that {{"2*frequency*(2*harmonics-1)"|codelit}} never goes over half your chosen sampling rate.
inputs: null
outputs:
  - [1, "A traignle wave."]
doc_name: additive triangle
doc_description: |
  The most accurate, least featureful, and slowest triangle oscillator.
  
  This oscillator uses additive synthesis to produce triangle waves.
  The efficiency therefore depends on the frequency.
  Sweeping this oscillator will perform poorly if you do not set the harmonics to a nonzero value.
  
  This oscillator is slightly under the range -1 to 1.
  Calibration scripts show that the worst case is -0.9 to 0.9, with the error increasing as frequency increases.
  For this reason, it is probably not suitable as a control signal.
  This is not fixable using additive synthesis and is a frequency dependent effect.