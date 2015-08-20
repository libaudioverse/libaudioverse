properties:
  Lav_DELAY_DELAY:
    name: delay
    type: float
    default: 0.0
    range: dynamic
    doc_description: |
      The delay of the delay line in seconds.
      The range of this property depends on the maxDelay parameter to the constructor.
      
      Note that values less than 1 sample still introduce delay.
      
      If you are going to connect nodes to or otherwise automate one of the delay properties, you need to use this one.
      The delay_samples property exists for advanced DSP applications that sometimes need sample-perfect accuracy.
      Changing delay_samples will cancel automators on this property.
  Lav_DELAY_DELAY_SAMPLES:
    name: delay_samples
    type: double
    range: dynamic
    default: 0.0
    doc_description: |
      The delay of the delay line in samples.
      
      This property's range depends on the maximum delay.  Values of 0 will introduce delay of 1 sample.
      
      This property is a double  so that it can accurately reflect the delay property.
      Setting it to a fractional value will make it arbitrarily pick one of the adjacent samples.
      
      Note that changing this property has the same effect as writing to the delay property: automation will be cancelled for both.
      You should use this property for one-off delay changes, as automation and node connection will behave unpredictably.
  Lav_DELAY_FEEDBACK:
    name: feedback
    type: float
    default: 0.0
    range: [-INFINITY, INFINITY]
    doc_description: |
      The feedback coefficient.
      The output of the delay line is fed back into the delay line, multiplied by this coefficient.
      Setting feedback to small values can make echoes, comb filters, and a variety of other effects.
  Lav_DELAY_INTERPOLATION_TIME:
    name: interpolation_time
    type: float
    default: 0.001
    range: [0.001, INFINITY]
    doc_description: |
      When the delay property is changed, the delay line crossfades between the old position and the new one.
      This property sets how long this crossfade will take.
      Note that for this node, it is impossible to get rid of the crossfade completely.
  Lav_DELAY_DELAY_MAX:
    name: delay_max
    type: float
    read_only: true
    doc_description: |
      The max delay as set at the node's creation time.
inputs:
  - [constructor, "The signal to delay."]
outputs:
  - [constructor, "The delayed signal."]
doc_name: crossfading delay line
doc_description: |
  Implements a crossfading delay line.
  Delay lines have uses in echo and reverb, as well as many more esoteric effects.