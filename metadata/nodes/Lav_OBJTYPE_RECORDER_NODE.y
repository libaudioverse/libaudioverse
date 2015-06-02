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