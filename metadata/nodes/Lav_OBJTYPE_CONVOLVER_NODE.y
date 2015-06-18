properties:
  Lav_CONVOLVER_IMPULSE_RESPONSE:
    name: impulse_response
    type: float_array
    min_length: 1
    max_length: MAX_INT
    range: [-INFINITY, INFINITY]
    default: [1.0]
    doc_description: |
      The impulse response to convolve the input with.
inputs:
  - [constructor, "The signal to be convolved."]
outputs:
  - [constructor, "The convolved signal."]
doc_name: convolver
doc_description: |
  A simple convolver.
  
  This implements convolution directly, without use of the FFT.