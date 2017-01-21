node(identifier = "Lav_OBJTYPE_BUFFER_NODE",
doc_name = "buffer",
This node plays a buffer.
The output of this node will have as many channels as the buffer does, so connecting it directly to the server will have the desired effect.
doc_description = """
"""
components = [
property(name = "buffer", identifier = "Lav_BUFFER_BUFFER", type = p_buffer, doc = """
The currently playing buffer.
Setting this property will reset position."""),
property(name = "position", identifier = "Lav_BUFFER_POSITION", type = P_double, range_type = dynamic,
    range = "The length of the currently set buffer, in seconds",
    doc = """
The position of playback, in seconds.
"""),
  property(name = "rate", identifier = "Lav_BUFFER_RATE", type = p_double, default = 1.0, range = (0.0, inf), doc = """
A multiplier that applies to playback rate.
1.0 is identity.
Values less than 1.0 cause a decrease in pitch and values greater than 1.0 cause an increase in pitch."""),
property(name = "looping", identifier = "Lav_BUFFER_LOOPING", ty[pe = p_boolean, doc = """
If true, this node continues playing the same buffer from the beginning after it reaches the end.
"""),
property(name = "ended_count", identifier = "Lav_BUFFER_ENDED_COUNT", type = p_int, range = (0, inf), doc = """
Increments every time the buffer reaches it's end.
If the buffer is not looping, this can be used to determine when the buffer is ended, without using the callback.
If the buffer is configured to loop, the counter will count up every time the end of a loop is reached.
You can write to this property to reset it.
""),
callback(name = "end", getter = "Lav_bufferNodeGetEndCallback", setter = "Lav_bufferNodeSetEndCallback", in_audio_thread = False, doc = """
Called every time the buffer reaches the end of the audio data.
"""),
output(channel_type = dynamic, channels = "Depends on the currently playing buffer", doc = "The buffer, if one is set. Otherwise silence."),
])