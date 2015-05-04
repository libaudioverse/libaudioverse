properties:
 Lav_PANNER_AZIMUTH: {name: azimuth, type: float, default: 0.0, range: [-INFINITY, INFINITY]}
 Lav_PANNER_ELEVATION: {name: elevation, type: float, default: 0.0, range: [-90.0, 90.0]}
 Lav_PANNER_SHOULD_CROSSFADE: {name: should_crossfade, type: boolean, default: 1}
 Lav_PANNER_STRATEGY: {name: strategy, default: Lav_PANNING_STRATEGY_STEREO, type: int, value_enum: Lav_PANNING_STRATEGIES}
doc_name: Multipanner