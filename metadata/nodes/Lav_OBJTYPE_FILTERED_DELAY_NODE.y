properties:
  Lav_FILTERED_DELAY_DELAY:
    name: delay
    type: float
    default: 0.0
    range: dynamic
    doc_description: |
      The delay of the delay line in seconds.
      The range of this property depends on the maxDelay parameter to the constructor.
  Lav_FILTERED_DELAY_FEEDBACK:
    name: feedback
    type: float
    default: 0.0
    range: [-INFINITY, INFINITY]
    doc_description: |
      The feedback coefficient.
      The output of the filter is fed back into the delay line, multiplied by this coefficient.
  Lav_FILTERED_DELAY_INTERPOLATION_TIME:
    name: interpolation_time
    type: float
    default: 0.001
    range: [0.001, INFINITY]
    doc_description: |
      When the delay property is changed, the delay line crossfades between the old position and the new one.
      This property sets how long this crossfade will take.
      Note that for this node, it is impossible to get rid of the crossfade completely.
  Lav_FILTERED_DELAY_DELAY_MAX:
    name: delay_max
    type: float
    read_only: true
    doc_description: |
      The max delay as set at the node's creation time.
  Lav_FILTERED_DELAY_FILTER_TYPE:
    name: filter_type
    type: int
    default: Lav_BIQUAD_TYPE_LOWPASS
    value_enum: Lav_BIQUAD_TYPES
    doc_description: |
      The type of the filter.
      This determines the interpretations of the other properties on this node.
  Lav_FILTERED_DELAY_Q:
    name: q
    type: float
    range: [0.001, INFINITY]
    default: 0.5
    doc_description: |
      Q is a mathematically complex parameter, a full description of which is beyond the scope of this manual.
      For lowpass, bandpass, and high pass filters, Q can be interpreted as a measure of resonation.
      For Q<=0.5, the filter is said to be damped:
      it will cut frequencies.
      For Q>0.5, however, some frequencies are likely to be boosted.
      
      Q controls the bandwidth for the bandpass and peaking filter types
      as well as the slope for the shelving EQ.
      
      For everything except the peaking filter, this property follows the normal definition of Q in the electrical engineering literature.
      For more specifics, see the Audio EQ Cookbook.
      It is found here:
      http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
  Lav_FILTERED_DELAY_FREQUENCY:
    name: frequency
    type: float
    range: [0, INFINITY]
    default: 2000.0
    doc_description: |
      This is the frequency of interest.
      What specifically this means depends on the selected filter type; for example, it is the cutoff frequency for lowpass and highpass.
  Lav_FILTERED_DELAY_DBGAIN:
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
  - [constructor, "The result of processing the audio via the filter and delay line."]
doc_name: filtered delay
doc_description: |
  This node consists of a delay line with a biquad filter attached.
  The output of the delay line is filtered.
  The difference between this node and a delay line and filter pair is that this node will use the filtered output for the feedback.
  
  This node is equivalent to the delay line in the Karplus-strong algorithm.