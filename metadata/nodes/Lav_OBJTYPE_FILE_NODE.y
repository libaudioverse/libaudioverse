properties:
 Lav_FILE_POSITION: {name: position, type: double, default: 0.0, range: dynamic}
 Lav_FILE_PITCH_BEND: {name: pitch_bend, type: float, default: 1.0, range: [0, INFINITY]}
 Lav_FILE_LOOPING: {name: looping, type: boolean, default: 0}
events:
 Lav_FILE_END_EVENT: {name: end}
doc_name: File