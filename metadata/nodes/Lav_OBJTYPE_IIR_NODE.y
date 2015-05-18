extra_functions:
  Lav_iirNodeSetCoefficients:
    name: set_coefficients
inputs:
  - [constructor, "The signal to filter."]
outputs:
  - [constructor, "The signal with the IIR filter applied."]
doc_name: IIR filter
doc_description: |
  Implements arbetrary IIR filters.
  The only restriction on the filter is that the first element of the denominator must be nonzero.
  To configure this node, use the function Lav_iirNodeSetCoefficients.