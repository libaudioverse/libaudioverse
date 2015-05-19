properties:
  Lav_PUSH_THRESHOLD:
    name: threshold
    type: float
    range: [0.0, INFINITY]
    default: 0.03
    doc_description: |
      When the remaining audio in the push node has a duration less than this property, the audio event fires.
events:
  Lav_PUSH_AUDIO_EVENT:
    name: audio
    multifiring_protection: true
    doc_description: |
      Fired when the remaining duration of the audio in this node is less than the threshold property.
  Lav_PUSH_OUT_EVENT:
    name: out
    doc_description: |
      Fired when the node has no more audio at all.
extra_functions:
  Lav_pushNodeFeed:
    doc_description: |
      Feed more audio data into the internal queue.
    params:
      length: The length of the buffer, in samples.
      frames: The buffer to feed into the node.  This memory is copied.
inputs: null
outputs:
  - [constructor, "Either audio from the internal queue or zero."]
doc_name: push
doc_description: |
  The purpose of this node is the same as the pull node, but it is used in situations wherein we do not know when we are going to get audio.
  Audio is queued as it is pushed to this node and then played as fast as possible.
  This node can be used to avoid writing a queue of audio yourself, as it essentially implements said functionality.
  If you need low latency audio or the ability to run something like the Opus encoder's
  ability to cover for missing frames, you need a pull node.