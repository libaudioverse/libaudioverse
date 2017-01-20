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
inputs:
  - [1, "The signal to pan."]
outputs:
  - [2, "The signal with the HRTF applied."]
doc_name: HRTF
doc_description: |
  This node implements an HRTF panner.
  You can use either Libaudioverse's internal HRTF (The Diffuse MIT Kemar Dataset) by passing "default" as the HRTf file name,
  or an HRTF of your own.