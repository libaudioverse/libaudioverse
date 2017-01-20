callbacks:
  audio:
    doc_description: |
      Called when the node needs more audio.
    params:
      frames:  The number of frames of audio needed.  This is not guaranteed to be the same on every call.
      channels: The number of channels as set when the pull node is created.
      buffer: The destination to which audio should be written.  This is a buffer that is {{"frames*channels"|codelit}} long.  Write interleaved audio data to it and do not assume that it is zeroed.
inputs: null
outputs:
  - [constructor, "The result of the configured callback."]
doc_name: pull
doc_description: |
  This node calls the audio callback whenever it needs more audio.
  The purpose of this node is to inject audio from an external source that Libaudioverse does not support, for example a custom network protocol.