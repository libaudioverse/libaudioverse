properties:
  Lav_BUFFER_BUFFER:
    name: buffer
    type: buffer
    doc_description: |
      The currently playing buffer.
      Setting this property will reset position.
  Lav_BUFFER_POSITION:
    name: position
    type: double
    default: 0.0
    range: dynamic
    doc_description: |
      The position of playback, in seconds.
      The range of this property corresponds the the total duration of the buffer.
  Lav_BUFFER_PITCH_BEND:
    name: pitch_bend
    type: float
    default: 1.0
    range: [0, INFINITY]
    doc_description: |
      A multiplier that applies to playback rate.
      1.0 is identity.
      Values less than 1.0 cause a decrease in pitch and values greater than 1.0 cause an increase in pitch.
      This corresponds directly with the doppler coefficient.
  Lav_BUFFER_LOOPING:
    name: looping
    type: boolean
    default: 0
    doc_description: |
      If true, this node continues playing the same buffer from the beginning after it reaches the end.
events:
  Lav_BUFFER_END_EVENT:
    name: end
    doc_description: |
      Fires each time this node reaches the end of the associated buffer.
inputs: null
outputs:
  - [ dynamic, "Depends on the currently playing buffer.", "The output from the buffer being played."]
doc_name: buffer
doc_description: |
  This node plays a buffer.
  The output of this node will have as many channels as the buffer does, so connecting it directly to the simulation will have the desired effect.