PROPERTIES:
 Lav_OBJTYPE_GENERIC:
  Lav_OBJECT_SUSPENDED: {name: suspended, type: float, default: 0, range: [0, 1]}
 Lav_OBJTYPE_SINE:
  Lav_SINE_FREQUENCY: {name: frequency, type: int, default: 440.0f, range: [0, INFINITY]}
 Lav_OBJTYPE_FILE:
  Lav_FILE_POSITION: {name: position, type: float, default: 0.0f, range: [0.0f, 0.0f]}
  Lav_FILE_PITCH_BEND: {name: pitch_bend, type: float, default: 1.0f, range: [0, INFINITY]}
 Lav_OBJTYPE_HRTF:
  Lav_HRTF_AZIMUTH: {name: azimuth, type: float, default: 0.0f, range: [-INFINITY, INFINITY]}
  Lav_HRTF_ELEVATION: {name: elevation, type: float, default: 0.0f, range: [-90.0f, 90.0f]}
 Lav_OBJTYPE_ATTENUATOR:
  Lav_ATTENUATOR_MULTIPLIER: {name: multiplier, type: float, default: 1.0f, range: [0.0f, INFINITY]}
 Lav_OBJTYPE_MIXER:
  Lav_MIXER_MAX_PARENTS: {name: max_parents, type: int, default: 0, range: [0, MAX_INT]}
  Lav_MIXER_INPUTS_PER_PARENT: {name: inputs_per_parent, type: int, default: 0, range: [0, 0]}
 Lav_OBJTYPE_WORLD:
  Lav_3D_POSITION: {name: position, type: float3, default: [0.0, 0.0, 0.0]}
  Lav_3D_ORIENTATION: {name: orientation, type: float6, default: [0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f]}
 Lav_OBJTYPE_SOURCE:
  Lav_3D_POSITION: {name: position, type: float3, default: [0.0, 0.0, 0.0]}
  Lav_3D_ORIENTATION: {name: orientation, type: float6, default: [0.0, 0.0, -1.0, 0.0, 1.0, 0.0]}
  Lav_SOURCE_MAX_DISTANCE: {name: max_distance, type: float, default: 50.0f, range: [0.0f, INFINITY]}
  Lav_SOURCE_DISTANCE_MODEL: {name: distance_model, type: int, default: Lav_DISTANCE_MODEL_LINEAR, range: [Lav_DISTANCE_MODEL_MIN, Lav_DISTANCE_MODEL_MAX]}
