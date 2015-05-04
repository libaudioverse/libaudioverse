properties:
 Lav_BIQUAD_FILTER_TYPE:
  name: filter_type
  type: int
  default: Lav_BIQUAD_TYPE_LOWPASS
  value_enum: Lav_BIQUAD_TYPES
 Lav_BIQUAD_Q:
  name: q
  type: float
  range: [0.001, INFINITY]
  default: 0.7
 Lav_BIQUAD_FREQUENCY:
  name: frequency
  type: float
  range: [0, INFINITY]
  default: 2000.0
 Lav_BIQUAD_DBGAIN:
  name: dbgain
  type: float
  range: [-INFINITY, INFINITY]
  default: 0.0
doc_name: Biquad Filter