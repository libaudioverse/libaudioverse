properties:
 Lav_3D_POSITION: {name: position, type: float3, default: [0.0, 0.0, 0.0]}
 Lav_3D_ORIENTATION: {name: orientation, type: float6, default: [0.0, 0.0, -1.0, 0.0, 1.0, 0.0]}
 Lav_SOURCE_MAX_DISTANCE: {name: max_distance, type: float, default: 50.0, range: [0.0, INFINITY]}
 Lav_SOURCE_SIZE: {name: size, type: float, range: [0.0, INFINITY], default: 1.0}
 Lav_SOURCE_DISTANCE_MODEL: {name: distance_model, type: int, default: Lav_DISTANCE_MODEL_LINEAR, value_enum: Lav_DISTANCE_MODELS}
 Lav_SOURCE_PANNER_STRATEGY: {name: panner_strategy, default: Lav_PANNING_STRATEGY_STEREO, value_enum: Lav_PANNING_STRATEGIES, type: int}
doc_name: Simple Source