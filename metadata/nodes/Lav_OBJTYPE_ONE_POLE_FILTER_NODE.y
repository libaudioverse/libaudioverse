properties:
  Lav_ONE_POLE_FILTER_FREQUENCY:
    name: frequency
    type: float
    range: dynamic
    default: 500.0
    rate: a
    doc_description: |
      The -3 DB frequency of the filter.
      
      The range of this property is from 0 to half the sampling rate.
  Lav_ONE_POLE_FILTER_IS_HIGHPASS:
    name: is_highpass
    type: boolean
    default: 0
    doc_description: |
      True if the filter is a highpass.
      
      If this property is false, the filter is a lowpass.
inputs:
  - [constructor, "The signal to filter."]
outputs:
  - [constructor, "The filtered signal."]
doc_name: One-Pole Filter
doc_description: |
  A one-pole filter section, implementing the transfer function {{"H(Z) = \frac{1}{1+A_0 Z^{-1} }"|latex}}
  
  This filter is capable of implementing either a lowpass or highpass filter and is extremmely cheep.
  The produced filter rolls off at about 6 DB per octave.
  
  The default filter configuration is a lowpass at 500 HZ.
  The type of the filter is controlled via the {{"is_highpass"|codelit}} property.
  If said property is true, the filter becomes a highpass.
  
  Note that this filter can be swept with a-rate accuracy.