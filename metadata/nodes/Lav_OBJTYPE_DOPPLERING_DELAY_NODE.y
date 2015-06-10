properties:
  Lav_DELAY_DELAY:
    name: delay
    type: float
    default: 0.0
    range: dynamic
    doc_description: |
      The delay of the delay line in seconds.
      The range of this property depends on the maxDelay parameter to the constructor.
  Lav_DELAY_INTERPOLATION_DELTA:
    name: interpolation_delta
    type: float
    default: 1.0
    range: [0.00, INFINITY]
    doc_description: |
      When the delay property is changed, the delay line moves from the old position to the new one.
      This property controls how fast the delay head can be moved.
      Higher values cause more noticeable pitch changes.
      
      Put simply, this is the number of additional samples that the delay head is allowed to jump in order to "catch up" with the  new position.
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
doc_name: dopplering delay line
doc_description: |
  Implements a dopplering delay line.
  Delay lines have uses in echo and reverb, as well as many more esoteric effects.