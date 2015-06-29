properties:
  Lav_LATE_REFLECTIONS_T60:
    name: t60
    type: float
    default: 0.3
    range: [0.05, INFINITY]
    doc_description: |
      The time it takes for the late reflections to decay by {{"-60 DB"|codelit}}.
      
      Higher values are indicative of larger spaces.
  Lav_LATE_REFLECTIONS_DENSITY:
    name: density
    default: 0.5
    type: float
    range: [0.0, 1.0]
    doc_description: |
      Controls the number of reflections heard per second.
      
      Higher values are generally indicative of more complex and reflective surfaces, placed closer together.
      As an example, a carpetted cathedral would have a lower value than a small, concrete room.
  Lav_LATE_REFLECTIONS_HF_T60:
    type: float
    name: hf_t60
    default: 0.3
    range: [0.0, INFINITY]
    doc_description: |
      The decay time of the high frequency band.
  Lav_LATE_REFLECTIONS_LF_T60:
    type: float
    name: lf_t60
    default: 0.3
    range: [0.0, INFINITY]
    doc_description: |
      The decay time for the reflections in the low frequency band.
  Lav_LATE_REFLECTIONS_HF_REFERENCE:
    type: float
    name: hf_reference
    default: 1000.0
    range: dynamic
    doc_description: |
      Frequencies above this value are in the "high band" of the late reflections.
      
      Frequencies below this value, but above {{"lf_reference"|codelit}} are in the middle band of the late reflections.
      
      The range of this property will always range from 0 to nyquist, or half the sampling rate of the simulation.
  Lav_LATE_REFLECTIONS_LF_REFERENCE:
    type: float
    name: lf_reference
    default: 500.0
    range: dynamic
    doc_description: |
      Frequencies below this valuea are in the "low band" of the late reflections.
      
      Frequencies above this value but below {{"hf_reference"|codelit}} are in the middle band of the late reflections.
      
      This property ranges from 0 to nyquist, or half the sampling rate.
  Lav_LATE_REFLECTIONS_AMPLITUDE_MODULATION_DEPTH:
    name: amplitude_modulation_depth
    type: float
    default: 0.0
    range: [0.0, 1.0]
    doc_description: |
      Applies internal amplitude modulation to the reverb.
      
      The effect of this property for low values of {{"amplitude_modulation_frequency"|codelit}} is to add spin to the reverb.
      At higher values of {{"amplitude_modulation_frequency"|codelit}}, this property is best described as adding granularity;
      at still higher values of {{"amplitude_modulation_frequency"|codelit}}, strange harmonics are introduced into the reverb.
  Lav_LATE_REFLECTIONS_AMPLITUDE_MODULATION_FREQUENCY:
    name: amplitude_modulation_frequency
    type: float
    default: 10.0
    range: [-INFINITY, INFINITY]
    doc_description: |
      Controls the rate of change of the amplitude modulation.
      
      Higher values increase the rate of the circular effect, eventually transitioning to ring modulation.
      Negative values reverse the direction.
  Lav_LATE_REFLECTIONS_DELAY_MODULATION_DEPTH:
    name: delay_modulation_depth
    type: float
    default: 0.0
    range: [0.0, 1.0]
    doc_description: |
      Controls the emphasis provided by modulation of the internal delay lines.
  Lav_LATE_REFLECTIONS_DELAY_MODULATION_FREQUENCY:
    name: delay_modulation_frequency
    type: float
    default: 10.0
    range: [-INFINITY, INFINITY]
    doc_description: |
      Controls the frequency of the delay line modulation.
inputs: described
outputs: described
doc_name: late reflections
doc_description: |
  A late reflections generator.
  
  This node generates the late reflections portion of the Libaudioverse reverb using a 16-line FDN algorithm with control of the {{"t60"|codelit}} decay time across three configurable frequency bands.
  
  This node always has 16 inputs and 16 outputs, as it is primarily used internally.
  if you wish to use this node in your own code, perhaps the simplest is to feed the same signal to all channels.

  The inputs represent the inputs to the internal FDN, and the outputs represent the outputs.