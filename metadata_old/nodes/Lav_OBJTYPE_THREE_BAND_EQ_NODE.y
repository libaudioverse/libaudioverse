properties:
  Lav_THREE_BAND_EQ_HIGHBAND_DBGAIN:
    type: float
    name: highband_dbgain
    default: 0.0
    range: [-INFINITY, INFINITY]
    doc_description: |
      The gain to apply to the highest frequency band as decibals.
  Lav_THREE_BAND_EQ_HIGHBAND_FREQUENCY:
    name: highband_frequency
    type: float
    default: 1000.0
    range: dynamic
    doc_description: |
      The frequency that divides the middle band from the high band.
  Lav_THREE_BAND_EQ_MIDBAND_DBGAIN:
    name: midband_dbgain
    type: float
    default: 0.0
    range: [-INFINITY, INFINITY]
    doc_description: |
      The gain to apply to the middle band of the equalizer.
  Lav_THREE_BAND_EQ_LOWBAND_DBGAIN:
    name: lowband_dbgain
    type: float
    default: 0.0
    range: [-INFINITY, INFINITY]
    doc_description: |
      The gain of the lowest frequency band as decibals.
  Lav_THREE_BAND_EQ_LOWBAND_FREQUENCY:
    name: lowband_frequency
    type: float
    default: 300.0
    range: dynamic
    doc_description: |
      The frequency that divides the low and middle bands.
      
      This ranges from 0 to nyquist.
inputs: 
  - [constructor, "The signal to equalize."]
outputs:
  - [constructor, "The equalized signal."]
doc_name: three band eq
doc_description: |
  A three band equalizer.
  
  This node consists of a peaking filter and a highshelf filter in series, such that the frequency spectrum may be equalized in three, configurable bands.
  
  The lowest of these bands begins at {{"lowband_frequency"|codelit}} and continues down to {{"0 hz"|codelit}}.
  The highest is from {{"highband_frequency"|codelit}} and continues until nyquist.
  The middle is the remaining space between the low and high band.
  If the high band begins below the low band, behavior is undefined, but will almost certainly not do what you want.
  Libaudioverse does not check for this case.
  
  The slopes that this node institutes are not perfect and cannot increase effectively beyond a certain point.
  This is the least expensive of the Libaudioverse equalizers, and is sufficient for many simpler applications.