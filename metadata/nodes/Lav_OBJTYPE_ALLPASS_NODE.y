properties:
  Lav_ALLPASS_DELAY_SAMPLES:
    name: delay_samples
    type: int
    default: 1
    range: dynamic
    doc_description: |
      The delay of the delay line in samples.
      The range of this property depends on the maxDelay parameter to the constructor.
      
      Note that values less than 1 sample still introduce delay.
  Lav_ALLPASS_INTERPOLATION_TIME:
    name: interpolation_time
    type: float
    default: 0.001
    range: [0.001, INFINITY]
    doc_description: |
      When the delay_samples property is changed, the delay line crossfades between the old position and the new one.
      Essentially, this property sets how long it will take the allpass filter to settle after a delay change.
      Note that for this node, it is impossible to get rid of the crossfade completely.
  Lav_ALLPASS_DELAY_SAMPLES_MAX:
    name: delay_max
    type: int
    read_only: true
    doc_description: |
      The max delay as set at the node's creation time.
  Lav_ALLPASS_COEFFICIENT:
    name: coefficient
    type: float
    range: [-INFINITY, INFINITY]
    doc_description: |
      The coefficient of the allpass filter's transfer function.
      
      For those not familiar with digital signal processing, this controls how quickly the repeating echoes created by this filter decay.
inputs:
  - [constructor, "The signal to filter."]
outputs:
  - [constructor, "The filtered signal."]
doc_name: allpass
doc_description: |
  Implements a first-order allpass filter whose transfer function is {{"\\frac{c+Z^{-d} }{1 + cZ^{-d} }"|latex}} where {{"c"|codelit}} is the coefficient and {{"d"|codelit}} the delay in samples.
  
  This filter is useful in various reverb designs.