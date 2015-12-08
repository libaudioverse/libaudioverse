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
      The range of this property corresponds to the total duration of the buffer.
  Lav_BUFFER_RATE:
    name: rate
    type: double
    default: 1.0
    range: [0, INFINITY]
    doc_description: |
      A multiplier that applies to playback rate.
      1.0 is identity.
      Values less than 1.0 cause a decrease in pitch and values greater than 1.0 cause an increase in pitch.
  Lav_BUFFER_LOOPING:
    name: looping
    type: boolean
    default: 0
    doc_description: |
      If true, this node continues playing the same buffer from the beginning after it reaches the end.
  Lav_BUFFER_ENDED_COUNT:
    name: ended_count
    type: int
    default: 0
    read_only: true
    doc_description: |
      Increments every time the buffer reaches it's end.
      If the buffer is not looping, this can be used to determine when the buffer is ended, without using the callback.
      if the buffer is configured to loop, the counter will count up every time the end of a loop is reached.
      Note that this property can technically wrap if your buffer node manages to end 2147483647 times.
      This should be impossible, save for the most long-running applications and shortest meaningful buffers.
callbacks:
  end:
    doc_description: |
      Called outside the audio threads every time the buffer reaches the end of the audio data.
inputs: null
outputs:
  - [ dynamic, "Depends on the currently playing buffer.", "The output from the buffer being played."]
doc_name: buffer
doc_description: |
  This node plays a buffer.
  The output of this node will have as many channels as the buffer does, so connecting it directly to the simulation will have the desired effect.