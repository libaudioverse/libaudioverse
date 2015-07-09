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
      0 is horizontal and positive values are vertical.
  Lav_PANNER_SHOULD_CROSSFADE:
    name: should_crossfade
    type: boolean
    default: 1
    doc_description: |
      Whether or not this panner should crossfade.
      Lack of crossfading introduces audible artifacts when the panner is moved and you usually want this on.
  Lav_PANNER_STRATEGY:
    name: strategy
    default: Lav_PANNING_STRATEGY_STEREO
    type: int
    value_enum: Lav_PANNING_STRATEGIES
    doc_description: |
      What type of panning to use.
      Possibilities include HRTF, stereo, 5.1, and 7.1 speaker configurations.
      For something more nontraditional, use an amplitude panner.
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
inputs:
  - [1, "The signal to pan."]
outputs:
  - [dynamic, "Depends on the currently set panning strategy.", "The signal, panned according to the configured panning strategy."]
doc_name: multipanner
doc_description: |
  A panner which can have the algorithm it uses changed at runtime.
  The use for multipanners is for applications in which we may wish to change the speaker configuration at runtime.
  Capabilities include switching from HRTF to stereo and back, a useful property for games wherein the user might or might not be using headphones.