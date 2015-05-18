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
inputs:
  - [1, "The audio to enter the 3D environment."]
outputs: null
doc_name: simple source
doc_description: |
  The source node allows the spatialization of sound that passes through it.
  Sources have one input which is mono, to which a node should be connected.
  the audio from the input is spatialized according both to the source's properties and those on its environment, 
  and passed directly to the environment.
  Sources have no outputs.
  To hear a source, you must connect its environment to something instead.
  