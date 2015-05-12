properties:
  Lav_FDN_INTERPOLATION_TIME:
    name: interpolation_time
    type: float
    range: [0.001, INFINITY]
    default: 0.001
    doc_description: |
      The crossfade duration used for the internal delay lines of this network.
  Lav_FDN_MAX_DELAY:
    name: delay_max
    type: float
    read_only: true
    doc_description: |
      The maximum delay any of the internal delay lines may be set to.
extra_functions:
  Lav_feedbackDelayNetworkNodeSetFeedbackMatrix:
    name: set_feedback_matrix
  Lav_feedbackDelayNetworkNodeSetOutputGains:
    name: set_output_gains
  Lav_feedbackDelayNetworkNodeSetDelays:
    name: set_delays
  Lav_feedbackDelayNetworkNodeSetFeedbackDelayMatrix:
    name: set_feedback_delay_matrix
doc_name: Feedback Delay Network
doc_description: |
  Implements a feedback delay network.
  This is possibly the single-most complicated node in Libaudioverse, and full documentation of it goes well beyond the manual.
  Unless you know  what a  feedback delay network is and have a specific reason for using one, this node will probably not help you.
