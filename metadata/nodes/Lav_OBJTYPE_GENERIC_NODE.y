suppress_implied_inherit: true
properties:
  Lav_NODE_STATE:
    name: state
    type: int
    default: Lav_NODESTATE_PLAYING
    value_enum: Lav_NODE_STATES
    doc_description: |
      The node's state.  See the basics section in the Libaudioverse manual for details.
      The default is usually what you want.
  Lav_NODE_MUL:
    name: mul
    type: float
    default: 1.0
    range: [-INFINITY, INFINITY]
    rate: a
    doc_description: |
      After this node processes, the value to which mul is set is used as a multiplier on the result.
      The most notable effect of this is to change the node's volume.
      A variety of other uses exist, however, especially as regards to nodes which are connected to properties.
      Mul is applied before add.
  Lav_NODE_ADD:
    name: add
    type: float
    default: 0.0
    range: [-INFINITY, INFINITY]
    rate: a
    doc_description: |
      After mul is applied, we add the value to which this property is set to the node's result.
  Lav_NODE_CHANNEL_INTERPRETATION:
    name: channel_interpretation
    type: int
    value_enum: Lav_CHANNEL_INTERPRETATIONS
    default: Lav_CHANNEL_INTERPRETATION_SPEAKERS
    doc_description: |
      How to treat channel count mismatches.
      The default is to apply mixing matrices when possible.
      
      If set to {{"Lav_CHANNEL_INTERPRETATION_SPEAKERS"|enum}}, mixing matrices are applied to inputs.
      Otherwise, when set to {{"Lav_CHANNEL_INTERPRETATION_DISCRETE"|enum}}, they are not.
      
      This property is almost never needed.
inputs: null
outputs: null
doc_name: generic
doc_description: |
  The properties and functionality described here are available on every Libaudioverse node without exception.
  
  While an enumeration value is reserved for the generic node, it should not be possible to receive one publicly.  This identifier is only used internally.