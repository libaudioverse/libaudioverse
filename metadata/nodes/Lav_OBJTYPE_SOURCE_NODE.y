properties:
  Lav_3D_POSITION:
    name: position
    type: float3
    default: [0.0, 0.0, 0.0]
    doc_description: |
      The position of the source in world coordinates.
  Lav_3D_ORIENTATION:
    name: orientation
    type: float6
    default: [0.0, 0.0, -1.0, 0.0, 1.0, 0.0]
    doc_description: |
      The orientation of the source.
      This is not currently used.
      In future, it will be used for sound cones and filters on sources facing away.
      The interpretation is the same as that for the listener: the first 3 values are the direction of the front and the second 3 the direction of the top.
      Note that these must both be unit vectors and that they must be orthoganal.
      They are packed because, also like the listener, they must never be modified separately.
  Lav_SOURCE_MAX_DISTANCE:
    name: max_distance
    type: float
    default: 50.0
    range: [0.0, INFINITY]
    doc_description: |
      The maximum distance from the listener at which the source will be audible.
  Lav_SOURCE_SIZE:
    name: size
    type: float
    range: [0.0, INFINITY]
    default: 0.0
    doc_description: |
      The size of the source.
      Sources are approximated as spheres.
      The size is used to determine the closest point on the source to the listener, and is the radius of this sphere.
      Size currently has no other effect.
  Lav_SOURCE_DISTANCE_MODEL:
    name: distance_model
    type: int
    default: Lav_DISTANCE_MODEL_LINEAR
    value_enum: Lav_DISTANCE_MODELS
    doc_description: |
      The distance model determines how quickly sources get quieter as they move away from the listener.
  Lav_SOURCE_PANNER_STRATEGY:
    name: panner_strategy
    default: Lav_PANNING_STRATEGY_STEREO
    value_enum: Lav_PANNING_STRATEGIES
    type: int
    doc_description: |
      The strategy for the internal multipanner.
  Lav_SOURCE_HEAD_RELATIVE:
    name: head_relative
    type: boolean
    default: 0
    doc_description: |
      Whether or not to consider this source's position to always be relative to the listener.
      
      Sources which are head relative interpret their positions in the default coordinate system, relative to the listener.
      Positive x is right, positive y is up, and positive z is behind the listener.
      The orientation and position properties of an environment do not affect head relative sources, making them ideal for such things as footsteps and/or HUD effects that should be panned.
  Lav_SOURCE_REVERB_DISTANCE:
    name: reverb_distance
    type: float
    range: [0.0, INFINITY]
    default: 30.0
    doc_description: |
      The distance at which the source will only be heard through the reverb effect sends.
      
      If this source is not feeding any effect sends configured as reverbs, this property has no effect.
      
      For values greater than {{"Lav_SOURCE_MAX_DISTANCE"|property}}, the source will always be heard at least somewhat in the dry path.
      {{"Lav_SOURCE_DISTANCE_MODEL"|property}} controls how this crossfading takes place.
  Lav_SOURCE_MIN_REVERB_LEVEL:
    name: min_reverb_level
    type: float
    range: [0.0, 1.0]
    default: 0.15
    doc_description: |
      The minimum reverb level allowed.
      
      if a send is configured to be a reverb send, this is the minimum amount of audio that will be diverted to it.
      
      Behavior is undefined if this property is ever greater than the value you give to {{"Lav_SOURCE_MAX_REVERB_LEVEL"|property}}.
  Lav_SOURCE_MAX_REVERB_LEVEL:
    name: max_reverb_level
    type: float
    range: [0.0, 1.0]
    default: 0.6
    doc_description: |
      The maximum amount of audio to be diverted to reverb sends, if any.
      
      Behavior is undefined if this property is ever less than {{"Lav_SOURCE_MIN_REVERB_LEVEL"|property}}.
  Lav_SOURCE_OCCLUSION:
    type: float
    name: occlusion
    range: [0.0, 1.0]
    default: 0.0
    doc_description: |
      A scalar representing how occluded this source is.
      
      This property controls internal filters of the source that make occluded objects sound muffled.
      A value of 1.0 is a fully occluded source, which will be all but silent; a value of 0.0 has no effect.
      
      It is extremely difficult to map occlusion to a physical quantity.
      In the real world, occlusion depends on mass, density, molecular structure, and a huge number of other factors.
      Libaudioverse therefore chooses to use this scalar quantity and to attempt to do the right thing.
extra_functions:
  Lav_sourceNodeFeedEffect:
    doc_description: |
      Begin feeding the specified effect send.
    params:
      effect: The index of the effect send to feed.
  Lav_sourceNodeStopFeedingEffect:
    doc_description: |
      Stop feeding an effect send.
    params:
      effect: The send to stop feeding.
inputs:
  - [1, "The audio to enter the 3D environment."]
outputs: null
doc_name: source
doc_description: |
  The source node allows the spatialization of sound that passes through it.
  Sources have one input which is mono, to which a node should be connected.
  The audio from the input is spatialized according both to the source's properties and those on its environment, and passed directly to the environment.
  Sources have no outputs.
  To hear a source, you must connect its environment to something instead.
  
  Since the source communicates with the environment through a nonstandard mechanism, environments do not keep their sources alive.
  If you are in a garbage collected language, failure to hold on to the source nodes will cause them to go silent.