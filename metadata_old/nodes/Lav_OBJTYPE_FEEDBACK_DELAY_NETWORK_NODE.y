properties:
  Lav_FDN_OUTPUT_GAINS:
    name: output_gains
    type: float_array
    dynamic_array: true
    doc_description: |
      Allows control of the individual gains of the output.
      These gains do not apply to the feedback path and are only for controlling relative output levels.
      The array for this property allows any floating point values, and must be exactly `channels` long.
  Lav_FDN_DELAYS:
    name: delays
    type: float_array
    dynamic_array: true
    doc_description: |
      The lengths of the delay lines in seconds.
      This array must be {{"channels"|codelit}} long.
      All values must be positive and no more than the maximum delay specified to the constructor.
  Lav_FDN_MATRIX:
    name: matrix
    type: float_array
    dynamic_array: true
    doc_description: |
      The feedback matrix.
      
      A column vector is formed by reading all delay lines.
      This vector is multiplied by this matrix, and then fed back into the delay lines.
      
      The matrix is stored in row-major order.
      The supplied array must have a length equal to the square of the channels specified to the constructor.
  Lav_FDN_FILTER_TYPES:
    name: filter_types
    type: int_array
    dynamic_array: true
    value_enum: Lav_FDN_FILTER_TYPES
    doc_description: |
      Allows insertion of filters in the feedback paths.
      These filters can be individually enabled and disabled; the default is disabled.
  Lav_FDN_FILTER_FREQUENCIES:
    name: filter_frequencies
    type: float_array
    dynamic_array: true
    doc_description: |
      The frequencies of the filters.
      The range of this property is 0 to Nyquist, or half the sampling rate.
  Lav_FDN_MAX_DELAY:
    name: max_delay
    type: float
    read_only: true
    doc_description: |
      The maximum delay any of the internal delay lines may be set to.
inputs: described
outputs: described
doc_name: feedback delay network
doc_description: |
  Implements a feedback delay network.
  This is possibly the single-most complicated node in Libaudioverse, and full documentation of it goes well beyond the manual.
  Unless you know  what a  feedback delay network is and have a specific reason for using one, this node will probably not help you.

  This node has `n` inputs and outputs, where `n` is the `lines` parameter to the constructor.
  Each input and output pair represent the input and output of a specific delay line, respectively.
  