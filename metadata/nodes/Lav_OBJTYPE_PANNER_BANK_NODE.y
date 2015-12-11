properties:
  Lav_PANNER_AZIMUTH:
    name: azimuth
    type: float
    default: 0.0
    range: [-INFINITY, INFINITY]
    doc_description: |
      The horizontal angle of the panner, in degrees.
      0 is straight ahead and positive values are clockwise.
  Lav_PANNER_SHOULD_CROSSFADE:
    name: should_crossfade
    type: boolean
    default: 1
    doc_description: |
      Whether or not the panners should crossfade.
      Lack of crossfading introduces audible artifacts when the panner is moved.
  Lav_PANNER_STRATEGY:
    name: strategy
    default: Lav_PANNING_STRATEGY_STEREO
    type: int
    value_enum: Lav_PANNING_STRATEGIES
    doc_description: |
      What type of panning to use.
      Possibilities include HRTF, stereo, 5.1, and 7.1 speaker configurations.
  Lav_PANNER_BANK_SPREAD:
    default: 360.0
    type: float
    name: spread
    range: [0.0, 360.0]
    doc_description: |
      An angle in degrees.
      Defines a cone across which the panners in this bank are spread.
      
      This cone always inclues the endpoints, and the panners are separated as {{"spread/(panner_count-1)"|codelit}}.
      This implies that for {{"spread=360.0"|codelit}}, the first and last inputs are effectively the same.
  Lav_PANNER_BANK_COUNT:
    name: count
    type: int
    default: true
    read_only: true
    doc_description: |
      The number of panners being used by this node.
      This is the same as the number of inputs to the node.
  Lav_PANNER_BANK_IS_CENTERED:
    name: is_centered
    type: boolean
    default: 0
    doc_description: |
      If true, then the azimuth controls the center of the cone.
      Otherwise, the azimuth controls the left edge of the cone.
inputs: constructor
outputs:
  - [dynamic, "Depends on the currently set panning strategy.", "The signal, panned according to the configured panning strategy."]
doc_name: panner bank
doc_description: |
  A bank of panners, separated to cover a configurable cone.
  
  The primary use for this node is to simulate virtual speakers.
  
  The number of panners is set by the {{"pannerCount"|param}} parameter to the constructor.
  This must be at least 2.
  Each input corresponds to a panner, starting with the panner set to the lowest angle.