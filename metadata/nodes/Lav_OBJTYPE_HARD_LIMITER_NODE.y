inputs:
  - [constructor, "The signal to limit"]
outputs:
  - [constructor, "The limited signal: no sample shall have an absolute value greater than 1.0."]
doc_name: hard limiter
doc_description: |
    The input to this node is hard limited: values less than -1.0 are set to -1.0 and values above 1.0 are set to 1.0.
    Use the hard limiter in order to prevent oddities with audio hardware; it should usually be the last piece in your pipeline before the simulation.
    Note that the 3D API handles hard limiting appropriately, and you do not need to worry about this there.