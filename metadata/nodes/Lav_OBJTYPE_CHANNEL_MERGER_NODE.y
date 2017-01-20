doc_name: channel merger
inputs: constructor
outputs:
  - [constructor, "The result of merging the input channels."]
doc_description: |
  Libaudioverse inputs and outputs transport multiple channels of audio, which is usually the desired behavior.
  This node, coupled with the {{"Lav_OBJTYPE_CHANNEL_SPLITTER_NODE"|node}} , allows advanced applications to manipulate the individual audio channels directly.
  The usual workflow is as follows: feed the output of a node through a {{"Lav_OBJTYPE_CHANNEL_SPLITTER_NODE"|node}},
  modify the channels individually, and then merge them with this node.