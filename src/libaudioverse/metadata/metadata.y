Lav_OBJTYPE_GENERIC:
 suppress_implied_inherit: true
 properties:
  Lav_OBJECT_STATE: {name: state, type: int, range: [0, 2], default: 1, value_enum: Lav_OBJECT_STATES}
  Lav_OBJECT_AUTORESET: {name: autoreset, type: boolean, default: 1}
 doc_name: Generic Properties Common to All Objects
Lav_OBJTYPE_SINE:
 properties:
  Lav_SINE_FREQUENCY: {name: frequency, type: float, default: 440.0, range: [0, INFINITY]}
 doc_name: Sine
Lav_OBJTYPE_FILE:
 properties:
  Lav_FILE_POSITION: {name: position, type: double, default: 0.0, range: [0.0, 0.0]}
  Lav_FILE_PITCH_BEND: {name: pitch_bend, type: float, default: 1.0, range: [0, INFINITY]}
  Lav_FILE_LOOPING: {name: looping, type: boolean, default: 0}
 callbacks:
  Lav_FILE_END_CALLBACK: {name: end}
 doc_name: File
Lav_OBJTYPE_HRTF:
 properties:
  Lav_PANNER_AZIMUTH: {name: azimuth, type: float, default: 0.0, range: [-INFINITY, INFINITY]}
  Lav_PANNER_ELEVATION: {name: elevation, type: float, default: 0.0, range: [-90.0, 90.0]}
  Lav_PANNER_SHOULD_CROSSFADE: {name: should_crossfade, type: boolean, default: 1}
 doc_name: Hrtf
Lav_OBJTYPE_AMPLITUDE_PANNER:
 properties:
  Lav_PANNER_AZIMUTH: {name: azimuth, type: float, default: 0.0, range: [-INFINITY, INFINITY]}
  Lav_PANNER_ELEVATION: {name: elevation, type: float, default: 0.0, range: [-90.0, 90.0]}
  Lav_PANNER_CHANNEL_MAP: {name: channel_map, type: float_array, min_length: 2, max_length: MAX_INT, default: [-90, 90]}
 doc_name: Amplitude Panner
Lav_OBJTYPE_ATTENUATOR:
 properties:
  Lav_ATTENUATOR_MULTIPLIER: {name: multiplier, type: float, default: 1.0, range: [0.0, INFINITY]}
 doc_name: Attenuator
Lav_OBJTYPE_MIXER:
 properties:
  Lav_MIXER_MAX_PARENTS: {name: max_parents, type: int, default: 0, range: [0, MAX_INT]}
  Lav_MIXER_INPUTS_PER_PARENT: {name: inputs_per_parent, type: int, default: 0, range: [0, 0]}
 doc_name: Mixer
Lav_OBJTYPE_WORLD:
 properties:
  Lav_3D_POSITION: {name: position, type: float3, default: [0.0, 0.0, 0.0]}
  Lav_3D_ORIENTATION: {name: orientation, type: float6, default: [0.0, 0.0, -1.0, 0.0, 1.0, 0.0]}
 doc_name: World
Lav_OBJTYPE_SOURCE:
 properties:
  Lav_3D_POSITION: {name: position, type: float3, default: [0.0, 0.0, 0.0]}
  Lav_3D_ORIENTATION: {name: orientation, type: float6, default: [0.0, 0.0, -1.0, 0.0, 1.0, 0.0]}
  Lav_SOURCE_MAX_DISTANCE: {name: max_distance, type: float, default: 50.0, range: [0.0, INFINITY]}
  Lav_SOURCE_DISTANCE_MODEL: {name: distance_model, type: int, default: 0, range: [0, 0], value_enum: Lav_DISTANCE_MODELS}
 doc_name: Simple Source
Lav_OBJTYPE_DELAY:
 properties:
  Lav_DELAY_DELAY: {name: delay, type: float, default: 0.001, range: [0.0, 0.0]}
  Lav_DELAY_DELAY_MAX: {name: delay_max, type: float, default: 1.0, range: [0.5, 5.0]}
  Lav_DELAY_FEEDBACK: {name: feedback, type: float, default: 0.0, range: [0.0, 1.0]}
  Lav_DELAY_INTERPOLATION_TIME: {name: interpolation_time, type: float, default: 0.001, range: [0.001, INFINITY]}
 doc_name: Delay Line
Lav_OBJTYPE_PUSH:
 properties:
  Lav_PUSH_THRESHOLD: {name: threshold, type: float, range: [0.0, INFINITY], default: 0.03}
 callbacks:
  Lav_PUSH_AUDIO_CALLBACK: {name: audio, multifiring_protection: true}
  Lav_PUSH_OUT_CALLBACK: {name: out}
 doc_name: Push Object
 extra_functions:
  Lav_pushObjectFeed: {name: feed}