inputs:
  - [constructor, "The signal whose gain is to be changed."]
outputs:
  - [constructor, "The signal with its gain changed."]
doc_name: gain
doc_description: |
  This node is essentially in instantiated generic node, offering only the functionality therein.
  Its purpose is to allow changing the gain or adding offset to a large collection of nodes.
  One possible use is as a simple mixer: point all the nodes to be mixed at the input, set mul, and then point the output at the destination for the mixed audio.