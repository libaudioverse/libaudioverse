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
      Disabling crossfading can aid performance under heavy workloads, especially with the HRTF panner.
      If crossfading is enabled, moving the panner will slowly fade it to the new position over the next block.
  Lav_PANNER_CHANNEL_MAP:
    name: channel_map
    type: float_array
    min_length: 2
    max_length: MAX_INT
    range: [-INFINITY, INFINITY]
    default: [-90, 90]
    doc_description: |
      The angles of the speakers in the order in which they are to be mapped to channels.
      The first speaker will be mapped to the first channel, the second to the second, etc.
      These channels are then combined and produced as the single output of the panner.
  Lav_PANNER_SKIP_LFE:
    name: skip_lfe
    type: boolean
    default: 1
    doc_description: |
      This property controls whether or not the LFE channel is reserved, i.e. skipped.
      You  almost always want this on.
  Lav_PANNER_HAS_LFE:
    name: has_lfe
    type: boolean
    default: 0
    doc_description: |
      Whether the current channel map has an LFE channel or not.
      If it does, this is assumed to be channel 4.
  Lav_PANNER_SKIP_CENTER:
    name: skip_center
    type: boolean
    default: 1
    doc_description: |
      This property controls whether or not the center channel is reserved, i.e. skipped.
      You  almost always want this on.
  Lav_PANNER_HAS_CENTER:
    name: has_center
    type: boolean
    default: 0
    doc_description: |
      Whether the currently specified channel map has a center channel.
      If it does, this is assumed to be channel 3.
  Lav_PANNER_PASSTHROUGH:
    name: passthrough
    type: float
    range: [0.0, 1.0]
    default: 0.0
    doc_description: |
      Controls how much of the "dry" signal is passed through.
      
      Higher values make the panning less obvious.
      
      Specifically, the input signal is added to all outputs with {{"passthrough"|codelit}} as the gain, and {{"1-passthrough"|codelit}} as the gain of the panned signal.
extra_functions:
  Lav_amplitudePannerNodeConfigureStandardMap:
    doc_description: |
      Sets the channel map and other properties on this node to match a standard configuration.
      The possible standard configurations are found in the {{"Lav_PANNING_STRATEGIES"|enum}} enumeration.
    params:
      channels: A value from the {{"Lav_PANNING_STRATEGIES"|enum}} enumeration.
inputs:
  - [1, "The signal to pan"]
outputs:
  - [dynamic, "Depends on a number of properties on this node.", "The result of panning the signal."]
doc_name: amplitude panner
doc_description: |
  This panner pans for a set of regular speakers without any additional effects applied.
  Additionally, it understands surround sound speaker layouts and allows for the assignment of custom speaker mappings.
  The default configuration provides a stereo panner that can be used without any additional steps.
  The additional function Lav_amplitudePannerNodeConfigureStandardChannelMap can set the panner to output for a variety of standard configurations, so be sure to see its documentation.
