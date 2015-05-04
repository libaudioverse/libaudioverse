doc_name: Buffer Node
properties:
 Lav_BUFFER_BUFFER: {name: buffer, type: buffer}
 Lav_BUFFER_POSITION: {name: position, type: double, default: 0.0, range: dynamic}
 Lav_BUFFER_PITCH_BEND: {name: pitch_bend, type: float, default: 1.0, range: [0, INFINITY]}
 Lav_BUFFER_LOOPING: {name: looping, type: boolean, default: 0}
events:
 Lav_BUFFER_END_EVENT: {name: end}