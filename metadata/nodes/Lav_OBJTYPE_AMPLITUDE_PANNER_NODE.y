properties:
 Lav_PANNER_AZIMUTH:
  name: azimuth
  type: float
  default: 0.0
  range: [-INFINITY, INFINITY]
  doc_description: |
   The horizontal angle of the panner in degrees.
   0 is directly ahead, and positive values are clockwise.
 Lav_PANNER_ELEVATION:
  name: elevation
  type: float
  default: 0.0
  range: [-90.0, 90.0]
  doc_description: |
   The vertical angle of the panner in degrees.
   0 is horizontal and positive values are upwards.
   Note that, for amplitude panners, this has no effect and exists only to allow swapping with the HRTF panner without changing code.
 Lav_PANNER_SHOULD_CROSSFADE:
  name: should_crossfade
  type: boolean
  default: 1
  doc_description: |
   Whether or not to instantly move to the new position.
   If crossfading is disabled, large movements of the panner will cause audible clicks.
   Disabling crossfading can aid performance under hevay workloads, especially with the HRTF panner.
   If crossfading is enabled, moving the panner will slowly fade it to the new position over the next block.
 Lav_PANNER_CHANNEL_MAP:
  name: channel_map
  type: float_array
  min_length: 2
  max_length: MAX_INT
  default: [-90, 90]
  doc_description: |
   The angles of the speakers in the order in which they are to be mapped to channels.
   The first speaker will be mapped to the first channel, the second to the second, etc.
   These channels are then combined and produced as the single output of the panner.
   
   Note that, in the case wherein this property is set to have more than 2 speakers, we assume that channel
   3 is the center and channel 4 is the LFE.
   By default, these are set to 0.
   If you are using this panner for a nontraditional purpose, i.e. panning across a set of delay lines,
   then set skip_lfe and skip_center to 0, to disable this functionality.
 Lav_PANNER_SKIP_LFE:
  name: skip_lfe
  type: boolean
  default: 1
  doc_description: |
   If more than 2 speakers are provided, this panner assumes that it is dealing with a surround sound layout.
   In order to make this work, the third and later speakers are shifted in order to leave silent channels for the center and LFE speakers.
   This property controls whether or not the LFE channel is reserved, i.e. skipped.
   You  almost always want this on.
 Lav_PANNER_SKIP_CENTER:
  name: skip_center
  type: boolean
  default: 1
  doc_description: |
   If more than 2 speakers are provided, this panner assumes that it is dealing with a surround sound layout.
   In order to make this work, the third and later speakers are shifted in order to leave silent channels for the center and LFE speakers.
   This property controls whether or not the center channel is reserved, i.e. skipped.
   You  almost always want this on.
extra_functions:
 Lav_amplitudePannerNodeConfigureStandardMap: {name: configure_standard_map}
doc_name: Amplitude Panner
doc_description: |
 This panner pans for a set of regular speakers without any additional effects applied.
 Additionally, it understands surround sound speaker layouts and allows for the assignment of custom speaker mappings.
 The default configuration provides a stereo panner that can be used without any additional steps.
 The additional function Lav_amplitudePannerNodeConfigureStandardChannelMap can set the panner to output for a variety of standard configurations, so be sure to see its documentation.
