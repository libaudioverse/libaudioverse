properties:
 Lav_3D_POSITION: {name: position, type: float3, default: [0.0, 0.0, 0.0]}
 Lav_3D_ORIENTATION: {name: orientation, type: float6, default: [0.0, 0.0, -1.0, 0.0, 1.0, 0.0]}
 Lav_ENVIRONMENT_DEFAULT_DISTANCE_MODEL: {name: default_distance_model, default: Lav_DISTANCE_MODEL_LINEAR, type: int, value_enum: Lav_DISTANCE_MODELS}
 Lav_ENVIRONMENT_DEFAULT_MAX_DISTANCE: {name: default_max_distance, range: [0.0, INFINITY], type: float, default: 50.0}
 Lav_ENVIRONMENT_DEFAULT_SIZE: {name: default_size, type: float, range: [0.0, INFINITY], default: 0.0}
 Lav_ENVIRONMENT_DEFAULT_PANNER_STRATEGY: {name: default_panner_strategy, type: int, default: 1, value_enum: Lav_PANNING_STRATEGIES}
 Lav_ENVIRONMENT_OUTPUT_CHANNELS: {name: output_channels, type: int, range: [0, MAX_INT], default: 2}
doc_name: Simple Environment