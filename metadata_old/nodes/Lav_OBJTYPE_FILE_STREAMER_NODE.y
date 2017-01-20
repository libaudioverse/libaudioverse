properties:
  Lav_FILE_STREAMER_POSITION:
    name: position
    type: double
    default: 0.0
    range: dynamic
    doc_description: |
      The position of playback, in seconds.
      The range of this property corresponds to the total duration of the file.
      Note that this property may be slightly inaccurate because this node has to pass data through a resampler.
  Lav_FILE_STREAMER_LOOPING:
    name: looping
    type: boolean
    default: 0
    doc_description: |
      If true, this node repeats the file from the beginning once it reaches the end.
      Note that setting looping means that ended will never go true.
      If ended is already true, it may take until the end of the next processing block for ended to properly go false once more.
  Lav_FILE_STREAMER_ENDED:
    name: ended
    type: boolean
    default: 0
    read_only: true
    doc_description: |
      Switches from false to true once the stream has ended completely and gone silent.
      This property will never go true unless looping is false.
callbacks:
  end:
    doc_description: |
      Called outside the audio threads after the stream has both reached its end and gone silent.
      When called, ended will be set to true,.
inputs: null
outputs:
  - [ dynamic, "Depends on the file.", "The output of the stream."]
doc_name: file streamer
doc_description: |
  Streams a file, which must be specified to the constructor and cannot be changed thereafter.
  
  This node is a stopgap solution, and should be considered temporary.
  It will likely remain for backward compatibility.
  Libaudioverse plans to eventually offer a more generic streaming node that also supports web addresses; such a node will have a completely different, less buffer-like interface.
  
  In order to stream a file, it must be passed through a resampler.
  Consequentlty, the position property is slightly inaccurate and the ended property and callback are slightly delayed.