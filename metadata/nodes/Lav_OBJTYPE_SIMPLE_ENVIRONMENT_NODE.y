properties:
  Lav_3D_POSITION:
    name: position
    type: float3
    default: [0.0, 0.0, 0.0]
    doc_description: |
      The position of the listener, in world coordinates.
  Lav_3D_ORIENTATION:
    name: orientation
    type: float6
    default: [0.0, 0.0, -1.0, 0.0, 1.0, 0.0]
    doc_description: |
      The orientation of the listener.
      The first three elements are a vector representing the direction in which the listener is looking
      and the second 3 a vector representing the direction in which a rod pointing out of the top of the listener's head would be pointing
      
      This property packs these vectors because they must never be modified separately.
      Additionally, they should both be unit vectors and must also be orthoganal.
      
      the default situates the listener such that positive x is right, positive y is up, and positive z is behind the listener.
      The setting (0, 1, 0, 0, 0, 1) will situate the listener such that
      positive x is right and positive y is forward.
      For those not familiar with trigonometry and who wish to consider positive x east and positivve y north, the following formula
      will turn the listener to face a scertain direction specified in radians clockwise of north:
      (sin(theta), cos(theta), 0, 0, 0, 1).
      As usual, note that radians=degrees*PI/180.
  Lav_ENVIRONMENT_DEFAULT_DISTANCE_MODEL:
    name: default_distance_model
    default: Lav_DISTANCE_MODEL_LINEAR
    type: int
    value_enum: Lav_DISTANCE_MODELS
    doc_description: |
      The default distance model for newly created sources.
      Distance models control how quickly sources get quieter as they move away from the listener.
  Lav_ENVIRONMENT_DEFAULT_MAX_DISTANCE:
    name: default_max_distance
    range: [0.0, INFINITY]
    type: float
    default: 50.0
    doc_description: |
      The default max distance for new sources.
      The max distance of a source is the maximum distance at which that source will be audible.
  Lav_ENVIRONMENT_DEFAULT_SIZE:
    name: default_size
    type: float
    range: [0.0, INFINITY]
    default: 0.0
    doc_description: |
      The default size for new sources.
      Sources aare approximated as spheres, with 0 being the special case of a point source.
      Size is used to determine the listener's distance from a source.
  Lav_ENVIRONMENT_DEFAULT_PANNER_STRATEGY:
    name: default_panning_strategy
    type: int
    default: 1
    value_enum: Lav_PANNING_STRATEGIES
    doc_description: |
      the default panner strategy for the internal panner of new sources.
  Lav_ENVIRONMENT_OUTPUT_CHANNELS:
    name: output_channels
    type: int
    range: [0, MAX_INT]
    default: 2
    doc_description: |
      Environments are not smart enough to determine the number of channels their output needs to have.
      If you are using something greater than stereo, i.e. 5.1, you need to change this property.
      The specific issue solved by this property is the case in which one source is set to something different than all others,
      or where the app changes the panning strategies of sources after creation.
doc_name: Simple Environment
doc_description: |
  This is the entry point to the 3D simulation capabilities.
  Environment nodes hold the information needed to pan sources, as well as acting as an aggregate output for all sources that use this environment.
  The simple environment is the simplest environment possible, having no functionality beyond positioning.
  
  Note that the various properties for default values do not affect already created sources.
  It is best to configure these first.
  Any functionality to change a property on all sources needs to be implemented by the app, and is not offered by Libaudioverse.