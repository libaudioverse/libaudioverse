doc_name: channel splitter
doc_description: |
  Libaudioverse inputs and outputs transport multiple channels of audio, which is usually the desired behavior.
  The channel splitter, coupled with the channel merger, allows advanced applications to manipulate the individual audio channels directly.
  The usual workflow is as follows: feed the output of a node through a channel splitter,
  modify the channels individually, and then merge them with a channel merger.