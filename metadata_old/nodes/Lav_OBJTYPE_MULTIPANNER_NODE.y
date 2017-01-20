properties:
  Lav_PANNER_AZIMUTH:
    name: azimuth
    type: float
    default: 0.0
    range: [-INFINITY, INFINITY]
    doc_description: |
      The horizontal angle of the panner, in degrees.
      0 is straight ahead and positive values are clockwise.
  Lav_PANNER_ELEVATION:
    name: elevation
    type: float
    default: 0.0
    range: [-90.0, 90.0]
    doc_description: |
      The vertical angle of the panner, in degrees.
      0 is horizontal and positive values are upward.
  Lav_PANNER_SHOULD_CROSSFADE:
    name: should_crossfade
    type: boolean
    default: 1
    doc_description: |
      Whether or not this panner should crossfade.
      Lack of crossfading introduces audible artifacts when the panner is moved.
      You usually want this on.
  Lav_PANNER_STRATEGY:
    name: strategy
    default: Lav_PANNING_STRATEGY_STEREO
    type: int
    value_enum: Lav_PANNING_STRATEGIES
    doc_description: |
      What type of panning to use.
      Possibilities include HRTF, stereo, 5.1, and 7.1 speaker configurations.
      For something more nontraditional, use an amplitude panner.
inputs:
  - [1, "The signal to pan."]
outputs:
  - [dynamic, "Depends on the currently set panning strategy.", "The signal, panned according to the configured panning strategy."]
doc_name: multipanner
doc_description: |
  A panner which can have the algorithm it uses changed at runtime.
  The use for multipanners is for applications in which we may wish to change the speaker configuration at runtime.
  Capabilities include switching from HRTF to stereo and back, a useful property for games wherein the user might or might not be using headphones.