properties:
 Lav_FDN_INTERPOLATION_TIME: {name: interpolation_time, type: float, range:[0.001, INFINITY], default: 0.001}
 Lav_FDN_MAX_DELAY: {name: delay_max, type: float, read_only: true}
extra_functions:
 Lav_feedbackDelayNetworkNodeSetFeedbackMatrix: {name: set_feedback_matrix}
 Lav_feedbackDelayNetworkNodeSetOutputGains: {name: set_output_gains}
 Lav_feedbackDelayNetworkNodeSetDelays: {name: set_delays}
 Lav_feedbackDelayNetworkNodeSetFeedbackDelayMatrix: {name: set_feedback_delay_matrix}
doc_name: Feedback Delay Network