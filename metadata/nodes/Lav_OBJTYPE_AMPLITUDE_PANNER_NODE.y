properties:
 Lav_PANNER_AZIMUTH: {name: azimuth, type: float, default: 0.0, range: [-INFINITY, INFINITY]}
 Lav_PANNER_ELEVATION: {name: elevation, type: float, default: 0.0, range: [-90.0, 90.0]}
 Lav_PANNER_SHOULD_CROSSFADE: {name: should_crossfade, type: boolean, default: 1}
 Lav_PANNER_CHANNEL_MAP: {name: channel_map, type: float_array, min_length: 2, max_length: MAX_INT, default: [-90, 90]}
 Lav_PANNER_SKIP_LFE: {name: skip_lfe, type: boolean, default: 1}
 Lav_PANNER_SKIP_CENTER: {name: skip_center, type: boolean, default: 1}
extra_functions:
 Lav_amplitudePannerNodeConfigureStandardMap: {name: configure_standard_map}
doc_name: Amplitude Panner