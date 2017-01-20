properties:
  Lav_BIQUAD_FILTER_TYPE:
    name: filter_type
    type: int
    default: Lav_BIQUAD_TYPE_LOWPASS
    value_enum: Lav_BIQUAD_TYPES
    doc_description: |
      The type of the filter.
      This determines the interpretations of the other properties on this node.
  Lav_BIQUAD_Q:
    name: q
    type: float
    range: [0.001, INFINITY]
    default: 0.5
    doc_description: |
      Q is a mathematically complex parameter, a full description of which is beyond the scope of this manual.
      Naively, Q can be interpreted as a measure of resonation.
      For {{"Q<=0.5"|codelit}}, the filter is said to be damped:
      it will cut frequencies.
      For Q>0.5, however, some frequencies are likely to be boosted.
      
      Q controls the bandwidth for the bandpass and peaking filter types
      For everything except the peaking filter, this property follows the normal definition of Q in the electrical engineering literature.
      For more specifics, see the Audio EQ Cookbook.
      It is found here:
      http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
  Lav_BIQUAD_FREQUENCY:
    name: frequency
    type: float
    range: [0, INFINITY]
    default: 2000.0
    doc_description: |
      This is the frequency of interest.
      What specifically this means depends on the selected filter type; for example, it is the cutoff frequency for lowpass and highpass.
  Lav_BIQUAD_DBGAIN:
    name: dbgain
    type: float
    range: [-INFINITY, INFINITY]
    default: 0.0
    doc_description: |
      This property is a the gain in decibals to be used with the peaking and shelving filters.
      It measures the gain that these filters apply to the part of the signal they boost.
inputs:
  - [constructor, "The signal to process."]
outputs:
  - [constructor, "The result of processing the audio via the filter."]
doc_name: biquad filter
doc_description: |
  Implementation of a biquad filter section, as defined by the Audio EQ Cookbook by Robert Bristo-Johnson.
  This node is capable of implementing almost every filter needed for basic audio effects, including equalizers.
  For the specific equations used, see the Audio EQ Cookbook.
  It may be found at:
  http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt