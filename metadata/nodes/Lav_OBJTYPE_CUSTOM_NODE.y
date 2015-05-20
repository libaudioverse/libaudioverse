callbacks: 
  processing:
    doc_description: |
      Called to process audio.
      If implementing a custom node, the custom node behaves as identity until this callback is set.
    params:
      frames: The number of frames to process.  This should always be the block size of the associated simulation.
      numInputs: The number of inputs of the node.
      inputs: An array of buffers representing the input audio.
      numOutputs: The number of outputs of the node.
      outputs: An array of buffers representing the audio output.
inputs: constructor
outputs: constructor
doc_name: custom
doc_description: |
  This node's processing depends solely on a user-defined callback.
  It has a specific number of inputs and outputs which are aggregated into individual channels.
  The callback is then called for every block of audio.
  If implementing a custom node, it is required that you handle all communication yourself.
