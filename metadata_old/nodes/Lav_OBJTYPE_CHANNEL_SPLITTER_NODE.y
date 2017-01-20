inputs:
  - [constructor, "The signal to split into component channels."]
outputs: constructor
doc_name: channel splitter
doc_description: |
  Libaudioverse inputs and outputs transport multiple channels of audio, which is usually the desired behavior.
  This node, coupled with the {{"Lav_OBJTYPE_CHANNEL_MERGER_NODE"|node}}, allows advanced applications to manipulate the individual audio channels directly.
  The usual workflow is as follows: feed the output of a node through this node,
  modify the channels individually, and then merge them with a {{"Lav_OBJTYPE_CHANNEL_MERGER_NODE"|node}}.