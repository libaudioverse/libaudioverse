properties:
 Lav_PUSH_THRESHOLD:
  name: threshold
  type: float
  range: [0.0, INFINITY]
  default: 0.03
events:
 Lav_PUSH_AUDIO_EVENT:
  name: audio
  multifiring_protection: true
 Lav_PUSH_OUT_EVENT:
  name: out

extra_functions:
 Lav_pushNodeFeed:
  name: feed
doc_name: Push Node