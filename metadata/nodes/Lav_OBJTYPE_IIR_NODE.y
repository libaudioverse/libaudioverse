extra_functions:
  Lav_iirNodeSetCoefficients:
    doc_description: |
      Configure the coefficients of the IIR filter.
    params:
      numeratorLength: The number of coefficients in the numerator of the filter's transfer function.
      numerator: The numerator of the transfer function.
      denominatorLength: The number of coefficients in the denominator of the transfer function.  Must be at least 1.
      denominator: The denominator of the transfer function.  The first coefficient must be nonzero.
      shouldClearHistory: 1 if we should reset the internal histories, otherwise 0.
inputs:
  - [constructor, "The signal to filter."]
outputs:
  - [constructor, "The signal with the IIR filter applied."]
doc_name: IIR filter
doc_description: |
  Implements arbetrary IIR filters.
  The only restriction on the filter is that the first element of the denominator must be nonzero.
  To configure this node, use the function Lav_iirNodeSetCoefficients.