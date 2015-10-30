inputs:
  - [constructor, "A signal with some DC."]
outputs:
  - [constructor, "The same signal with DC cut using a first-order filter."]
doc_name: dc blocker
doc_description: |
  A DC blocker.
  This is a first-order filter, the best possible within numerical limits.
  It consists of a zero at DC, and a pole as close to DC as we can put it.
  For any sampling rate, this node is the best first-order section for DC removal possible.