properties:
 Lav_OBJTYPE_GENERIC:
  Lav_OBJECT_SUSPENDED: {name: suspended, type: int, default: 0, range: [0, 1]}
 Lav_OBJTYPE_SINE:
  Lav_SINE_FREQUENCY: {name: frequency, type: float, default: 440.0, range: [0, INFINITY]}
 Lav_OBJTYPE_FILE:
  Lav_FILE_POSITION: {name: position, type: float, default: 0.0, range: [0.0, 0.0]}
  Lav_FILE_PITCH_BEND: {name: pitch_bend, type: float, default: 1.0, range: [0, INFINITY]}
 Lav_OBJTYPE_HRTF:
  Lav_HRTF_AZIMUTH: {name: azimuth, type: float, default: 0.0, range: [-INFINITY, INFINITY]}
  Lav_HRTF_ELEVATION: {name: elevation, type: float, default: 0.0, range: [-90.0, 90.0]}
 Lav_OBJTYPE_ATTENUATOR:
  Lav_ATTENUATOR_MULTIPLIER: {name: multiplier, type: float, default: 1.0, range: [0.0, INFINITY]}
 Lav_OBJTYPE_MIXER:
  Lav_MIXER_MAX_PARENTS: {name: max_parents, type: int, default: 0, range: [0, MAX_INT]}
  Lav_MIXER_INPUTS_PER_PARENT: {name: inputs_per_parent, type: int, default: 0, range: [0, 0]}
 Lav_OBJTYPE_WORLD:
  Lav_3D_POSITION: {name: position, type: float3, default: [0.0, 0.0, 0.0]}
  Lav_3D_ORIENTATION: {name: orientation, type: float6, default: [0.0, 0.0, -1.0, 0.0, 1.0, 0.0]}
 Lav_OBJTYPE_SOURCE:
  Lav_3D_POSITION: {name: position, type: float3, default: [0.0, 0.0, 0.0]}
  Lav_3D_ORIENTATION: {name: orientation, type: float6, default: [0.0, 0.0, -1.0, 0.0, 1.0, 0.0]}
  Lav_SOURCE_MAX_DISTANCE: {name: max_distance, type: float, default: 50.0, range: [0.0, INFINITY]}
  Lav_SOURCE_DISTANCE_MODEL: {name: distance_model, type: int, default: 0, range: [0, 0]}
 Lav_OBJTYPE_DELAY:
  Lav_DELAY_DELAY: {name: delay, type: float, default: 0.0, range: [0.0, 0.0]}
  Lav_DELAY_DELAY_MAX: {name: delay_max, type: float, default: 1.0, range: [0.0, 5.0]}
  Lav_DELAY_FEEDBACK: {name: feedback, type: float, default: 0.0, range: [0.0, 1.0]}
  Lav_DELAY_INTERPOLATION_TIME: {name: interpolation_time, type: float, default: 0.001, range: [0, INFINITY]}
