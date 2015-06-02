extra_functions:
  Lav_recorderNodeStartRecording:
    doc_description: |
      Begin recording to the specified files.
      The sample rate is the same as that of the simulation.
      The channel count is the same as this node was initialized with.
      The format of the file is determined from the extension: this function recognizes ".wav" and ".ogg" on all platforms.
    params:
      path: The path of the file to record to.
  Lav_recorderNodeStopRecording:
    doc_description: |
      Stops recording.
      
      Be sure to call this function.
      Failure to do so may lead to any of a number of undesirable problems.
inputs:
  - [constructor, "The signal to record."]
outputs:
  - [constructor, "The same as the input signal."]
doc_name: recorder
doc_description: |
  Records audio to files.
  
  The usage pattern for this node is simple:
  connect something to the input, call {{"Lav_recorderNodeStartRecording"|extra_function}}, and ensure that the node is processed.
  In order to avoid potential problems with blocks of silence at the beginning of the recording, this node's default state is playing.
  You should connect the output to something that will demand the recorder node's audio or change the state to always playing, usually after a call to {{"Lav_recorderNodeStartRecording"|extra_function}}.
  If you don't, no recording will take place.
  
  Unlike most other nodes in Libaudioverse, it is important that you call {{"Lav_recorderNodeStopRecording"|extra_function}} when done recording.
  Failure to do so may lead to any of a number of surprising results.