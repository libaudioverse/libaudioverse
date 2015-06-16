extra_functions:
  Lav_fftConvolverNodeSetResponse:
    doc_description: |
      Set the response for a specific channel.
    params:
      channel: The channel to set the response for.
      length: The length of the response in samples.
      response: The new response.
  Lav_fftConvolverNodeSetResponseFromFile:
    doc_description: |
      Set the impulse response for a specific channel of this node from a file.
    params:
      path: The path to the file.
      fileChannel: The channel of the file to use as the response.
      convolverChannel: The channel for which the response is to be set.
inputs:
  - [constructor, "The signal to be convolved."]
outputs:
  - [constructor, "The convolved signal."]
doc_name: fft convolver
doc_description: |
  A convolver for long impulse responses.
  
  This convolver uses the overlap-add convolution algorithm.
  It is slower than the {{"Lav_OBJTYPE_CONVOLVER_NODE"|node}} for small impulse responses.
  
  The difference between this node and the {{"Lav_OBJTYPE_CONVOLVER_NODE"|node}} is the complexity of the algorithm.
  This node is capable of handling impulses longer than a second, a case for which the {{"Lav_OBJTYPE_CONVOLVER_NODE"|node}} will fail to run in realtime.
  
  Furthermore, as the most common operation for this node is reverb, it is possible to set each channel's response separately.