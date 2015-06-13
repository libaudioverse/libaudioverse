properties:
  Lav_SQUARE_FREQUENCY:
    name: frequency
    type: float
    default: 440.0
    range: [0, INFINITY]
    doc_description: |
      The frequency of the square wave, in hertz.
  Lav_SQUARE_DUTY_CYCLE:
    name: duty_cycle
    type: float
    default: 0.5
    range: [0.0, 1.0]
    doc_description: |
      The duty cycle of the square wave.
      0 is always off and 1 is always on.
  Lav_SQUARE_PHASE:
    name: phase
    range: [0.0, 1.0]
    type: float
    default: 0.0
    doc_description: |
      The phase of the square wave.
      This is measured in periods, not in radians.
inputs: null
outputs:
  - [1, "A square wave."]
doc_name: square
doc_description: |
  A simple square wave oscillator.
  