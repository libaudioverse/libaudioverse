suppress_implied_inherit: true
properties:
 Lav_NODE_STATE:
  name: state
  type: int
  default: Lav_NODESTATE_PLAYING
  value_enum: Lav_NODE_STATES
 Lav_NODE_AUTORESET:
  name: autoreset
  type: boolean
  default: 1
 Lav_NODE_MUL:
  name: mul
  type: float
  default: 1.0
  range: [-INFINITY, INFINITY]
 Lav_NODE_ADD:
  name: add
  type: float
  default: 0.0
  range: [-INFINITY, INFINITY]
 Lav_NODE_CHANNEL_INTERPRETATION:
  name: channel_interpretation
  type: int
  value_enum: Lav_CHANNEL_INTERPRETATIONS
doc_name: Generic Properties Common to All Nodes
