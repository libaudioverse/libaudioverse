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
  Lav_PANNER_HEAD_WIDTH:
    type: float
    name: head_width
    default: 0.15
    range: [0.0, INFINITY]
    doc_description: |
      The width of the head of the listener, in meters.
      
      This property controls the HRTF panning strategy only.
  Lav_PANNER_SPEED_OF_SOUND:
    name: speed_of_sound
    type: float
    default: 440.0
    range: [1.0, INFINITY]
    doc_desccription: |
      The speed of sound, in meters per second.
      
      This property controls the HRTF panning strategy only.
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
      
      This property applies only to the HRTF panning strategy.
  Lav_PANNER_EAR_POSITION:
    name: ear_position
    default: 0.1
    range: [-1.0, 1.0]
    type: float
    doc_description: |
      The horizontal offset of the axis defined by the ears from that of the center of the head.
      
      This is represented as a multiplier of the head's radius, half the head's width.
      Positive values are backward and negative ones forward.
      
      This property applies only to the HRTF panning strategy.
  Lav_PANNER_BANK_SPREAD:
    default: 360.0
    type: float
    name: spread
    range: [0.0, 360.0]
    doc_descripption: |
      An angle in degrees.
      Defines a cone across which the panners in this bank are spread.
      
      This cone always inclues the endpoints, and the panners are separated as {{"spread/(panner_count-1)"|codelit}}.
      This implies that for {"spread=360.0"|codelit}}, the first and last inputs are effectively the same.
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
  Lav_PANNER_APPLY_ITD:
    name: apply_itd
    type: boolean
    default: 0
    doc_description: |
      Whether or not to apply the interaural time difference.
      
      When sound is heard, it will arrive at one ear before the other.
      This property controls whether or not to add additional delay to the panning effect in order to simulate this.
      If enabled, HRTFs become more computationally expensive, but allow for configurability via the {{"distance"|codelit}}, {{"head_width"|codelit}}, and {{"ear_position"|codelit}} properties.
      
      Changing this property will cause an audible glitch in audio.
      
      This property applies only to the HRTF panning strategy.
  Lav_PANNER_USE_LINEAR_PHASE:
    name: use_linear_phase
    type: boolean
    default: 0
    doc_desccription: |
      Whether or not to convert the HRTF to linear phase.
      
      This is typically used when {{"apply_idt"|codelit}} is enabled, in order to work properly with datasets that are not already converted to linear phase.
      
      Enabling this property adds 8 FFT/IFFT pairs to the computation of the HRIR.
      
      This property applies only to the HRTF panning strategy.
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