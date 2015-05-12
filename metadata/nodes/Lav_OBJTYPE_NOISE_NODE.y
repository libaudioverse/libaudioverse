properties:
  Lav_NOISE_NOISE_TYPE:
    name: noise_type
    type: int
    value_enum: Lav_NOISE_TYPES
    default: Lav_NOISE_TYPE_WHITE
    doc_description: |
      The type of noise to generate.
  Lav_NOISE_SHOULD_NORMALIZE:
    name: should_normalize
    type: boolean
    default: 0
    doc_description: |
      Whether or not to normalize the output.
      Some types of noise are quieter without this enabled.
      Turning it on is sometimes helpful and sometimes not.
doc_name: Noise Generator
doc_description: |
  Generates any of a variety of types of noise.