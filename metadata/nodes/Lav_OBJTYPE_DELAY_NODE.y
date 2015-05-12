properties:
  Lav_DELAY_DELAY:
    name: delay
    type: float
    default: 0.0
    range: dynamic
    doc_description: |
      The delay of the delay line in seconds.
      The range of this property depends on the maxDelay parameter to the constructor.
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
doc_name: Delay Line
doc_description: |
  Implements a crossfading delay line.
  Delay lines have uses in echo and reverb, as well as many more esoteric effects.