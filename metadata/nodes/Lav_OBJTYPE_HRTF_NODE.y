properties:
  Lav_PANNER_AZIMUTH:
    name: azimuth
    type: float
    default: 0.0
    range: [-INFINITY, INFINITY]
    doc_description: |
      The horizontal angle of the panner in degrees.
      0 is straight ahead and positive values are clockwise.
  Lav_PANNER_ELEVATION:
    name: elevation
    type: float
    default: 0.0
    range: [-90.0, 90.0]
    doc_description: |
      The vertical angle of the panner in degrees.
      0 is horizontal and positive values move upward.
  Lav_PANNER_SHOULD_CROSSFADE:
    name: should_crossfade
    type: boolean
    default: 1
    doc_description: |
      By default, panners crossfade their output.
      This property allows such functionality to be disabled.
      Note that for HRTF nodes, crossfading is more important than for other panner types.
      Unlike other panner types, the audio artifacts produced by disabling crossfading are noticeable, even for updates of only a few degrees.
  Lav_PANNER_HEAD_WIDTH:
    type: float
    default: 0.1
    name: head_width
    range: [0.0, INFINITY]
    doc_description: |
      The width of the head of the listener, in meters.
  Lav_PANNER_SPEED_OF_SOUND:
    name: speed_of_sound
    type: float
    default: 440.0
    range: [1.0, INFINITY]
    doc_desccription: |
      The speed of sound, in meters per second.
  Lav_PANNER_DISTANCE:
    name: distance
    type: float
    default: 1.0
    range: [0.0, INFINITY]
    doc_description: |
      The distance of the sound source from the listener, in meters.
      
      This property does not introduce attenuation.
      It is used only for computing interaural time differences, which are distance-dependent.
      
      This property is clamped to be greater than the head width at runtime.
inputs:
  - [1, "The signal to pan."]
outputs:
  - [2, "The signal with the HRTF applied."]
doc_name: HRTF
doc_description: |
  This node implements an HRTF panner.
  You can use either Libaudioverse's internal HRTF (The Diffuse MIT Kemar Dataset) by passing "default" as the HRTf file name,
  or an HRTF of your own.