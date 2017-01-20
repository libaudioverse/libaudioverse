properties:
  Lav_CROSSFADER_TARGET_INPUT:
    name: target_input
    read_only: true
    type: int
    default: 0
    range: dynamic
    doc_description: |
      The input which the current crossfade is headed for.
      
      When not crossfading, this property is meaningless.
  Lav_CROSSFADER_CURRENT_INPUT:
    name: current_input
    type: int
    default: 0
    range: dynamic
    doc_description: |
      The currently active input.
      
      Writing to this property is equivalent to crossfading with a time of 0.
      Note that the output is a combination of the current and target inputs while crossfading.
  Lav_CROSSFADER_IS_CROSSFADING:
    name: is_crossfading
    type: boolean
    default: 0
    doc_description: |
      True if we are crossfading, otherwise false.
extra_functions:
  Lav_crossfaderNodeCrossfade:
    doc_description: |
      Begin a crossfade.
      
      if a crossfade is currently inn progress, it finishes immediately and fires the event.
      
      Using a duration of 0 is an instantaneous crossfade, equivalent to writing directly to the current_input property.
      Crossfades of duration 0 do not fire the finished event.
    params:
      duration: The duration of the crossfade.
      input: The input to crossfade to.
callbacks:  
  finished:
    doc_description: |
      Called outside the audio thread when the currently scheduled crossfade finishes.
inputs: constructor
outputs:
  - [dynamic, "Depends on the channel count given to the constructor.", "The output of the crossfade."]
doc_name: crossfader
doc_description: |
  A crossfader is a node  which allows for selection of exactly one input.
  The selection can be changed by crossfading, a technique whereby the currently selected input is slowly faded out and the new one faded in.
  
  By using crossfades of no duration, this node can also be made to function as a switch or selector, selecting an input from among the connected nodes.
  This particular case is optimized, and special support is implemented via allowing you to write directly to the {{"current_input"|codelit}} property.
  
  This crossfader has a configurable number of inputs.
  All inputs and the single output have the same channel count.
  These are both configurable via parameters to the constructor.