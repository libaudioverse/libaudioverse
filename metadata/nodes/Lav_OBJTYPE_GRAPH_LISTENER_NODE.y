callbacks:
  listening:
    doc_description: |
      When set, audio is passed to this callback every block.
      This callback is called inside the audio threads; do not block.
    params:
      frames: The number of frames of audio being made available to client code.  This should always be the same as the simulation's block size.
      channels: The number of channels of audio being made available to client code.
      buffer: The data, stored in interleaved format.  The length of this buffer is {{"frames*channels"|codelit}}.
inputs:
  - [constructor, "The audio which will be passed to the associated callback."]
outputs:
  - [constructor, "The same audio as connected to the input without modification."]
doc_name: graph listener
doc_description: |
  This node defines a callback which is called every block.
  The callback is passed pointers to the audio data passing through this node for the current block.
  The effect is that this node allows observing audio passing through any location in the audio graph.