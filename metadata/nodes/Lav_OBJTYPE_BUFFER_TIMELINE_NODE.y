extra_functions:
  Lav_bufferTimelineNodeScheduleBuffer:
    doc_description: |
      Schedule a buffer, optionally with pitch bend.
      The time is relative to now.
    params:
      bufferHandle: The buffer to schedule.
      pitchBend: The pitch bend, which must be positive.  Use 1.0 to disable.
      time: The time to play the buffer.
inputs: null
outputs:
  - [constructor, "The sum of all playing buffers."]
doc_name: buffer timeline
doc_description: |
  Represents timelines of buffers.
  
  This node provides the ability to schedule buffers to play at any specific time in the future.
  This node supports pitch bending scheduled buffers.
  There is no limit to the number of buffers which may be scheduled at any given time, and polyphony is supported.