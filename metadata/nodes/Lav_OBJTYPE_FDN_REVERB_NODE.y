properties:
  Lav_FDN_REVERB_T60:
    name: t60
    type: float
    range: [0.0, INFINITY]
    default: 1.0
    doc_description: |
      The {{"t60"|codelit}} is the time it takes the reverb to decay by 60 decibals.
  Lav_FDN_REVERB_CUTOFF_FREQUENCY:
    name: cutoff_frequency
    type: float
    range: dynamic
    default: 5000
    doc_description: |
      Controls the frequencies of lowpass filters on the feedback path of the reverb.
      Lowering this property leads to softer and less harsh reverb.
  Lav_FDN_REVERB_DENSITY:
    name: density
    type: float
    range: [0.0, 1.0]
    default: 0.5
    doc_description: |
      Controls the density of the reverb.
      Extremely low values sound "grainy"; extremely high values tend to resonate.
  Lav_FDN_REVERB_DELAY_MODULATION_FREQUENCY:
    name: delay_modulation_frequency
    type: float
    default: 10.0
    range: [0.0, 500.0]
    doc_description: |
      Controls how fast the internal delay lines modulate.
  Lav_FDN_REVERB_DELAY_MODULATION_DEPTH:
    name: delay_modulation_depth
    type: float
    default: 0.0
    range: [0.0, 1.0]
    doc_description: |
      Controls how deep the modulation of the delay lines is.
      Increasing this property slightly makes the late reverb sound less metallic, while extremely high values add chorus-like effects.
      This property acts as a multiplier, and the correspondance between it and physical units is intentionally left unspecified.
inputs:
  - [4, "The signal to apply reverb to."]
outputs:
  - [4, "The signal with reverb applied."]
doc_name: fdn reverb
doc_description: |
  An 8 delay line FDN reverberator, based off a householder reflection.
  
  This reverb takes as its input and outputs ats its output quadraphonic audio.
  Panning effects will still be observed at the output with some bias.
  If a stereo signal is fed into the reverb and the reverb is likewise connected to a stereo output, the input signal's volume will effectively be halved.