node(identifier = "Lav_OBJTYPE_ENVIRONMENT_NODE",
doc_name = "Environment Node",
doc_description = """
This is the entry point to the 3D simulation capabilities.
Environment nodes hold the information needed to pan sources, as well as acting as an aggregate output for all sources that use this environment.
""",
components = [
    property(name = "position", identifier = "Lav_ENVIRONMENT_POSITION", type = p_float3, default_type = specified, doc = """
The position of the listener, in world coordinates.
"""),
    property(name = "orientation", identifier = "Lav_ENVIRONMENT_ORIENTATION", type = p_float6, default = [0.0, 0.0, -1.0, 0.0, 1.0, 0.0], doc = """
The orientation of the listener.
The first three elements are a vector representing the direction in which the listener is looking, and the second a vector representing an imaginary rod out of the top of the listener's head.

This property packs these vectors because they must never be modified separately.
Additionally, they should both be unit vectors and must also be orthoganal.

The default situates the listener such that positive x is right, positive y is up, and positive z is behind the listener.
The setting (0, 1, 0, 0, 0, 1) will situate the listener such that positive x is right and positive y is forward.
For those not familiar with trigonometry and who wish to consider positive x east and positivve y north, the following formula will turn the listener to face a certain direction specified in radians clockwise of north:
{{"(sin(theta), cos(theta), 0, 0, 0, 1)"|codelit}}.

As usual, note that {{"radians=degrees*PI/180"|codelit}}.
"""),
    property(name = "distance_model", identifier = "Lav_ENVIRONMENT_DISTANCE_MODEL", type = p_int, default = "Lav_DISTANCE_MODEL_LINEAR", associated_enum = "Lav_DISTANCE_MODELS", doc = """
Distance models control how quickly sources get quieter as they move away from the listener.

By default, sources are configured to delegate to the environment when looking for values to use for the distance model parameters.  This behavior may be changed by setting {{propref("Lav_OBJTYPE_SOURCE_NODE", "Lav_SOURCE_CONTROL_DISTANCE_MODEL")}} to true.
"""),
    property(name = "max_distance", identifier = "Lav_ENVIRONMENT_MAX_DISTANCE", type = p_float, range = (0.0, inf), default = 150.0, doc = """
The maximum distance at which a source is audible.
Consider this property to be in meters.

By default, sources are configured to delegate to the environment when looking for values to use for the distance model parameters.
This behavior may be changed by setting {{propref("Lav_OBJTYPE_SOURCE_NODE", "Lav_SOURCE_CONTROL_DISTANCE_MODEL")}} to true.
"""),
    property(name = "default_size", identifier = "Lav_ENVIRONMENT_DEFAULT_SIZE", type = p_float, range = (0.0, inf), default = 0.0, doc = """
The default size for new sources.
Sources aare approximated as spheres, with 0 being the special case of a point source.
Size is used to determine the listener's distance from a source.
This property is copied to sources upon creation, but changes to it thereafter do not affect already created sources.
"""),
    property(name = "panning_strategy", identifier = "Lav_ENVIRONMENT_PANNING_STRATEGY", type = p_int, default = "Lav_PANNING_STRATEGY_STEREO", associated_enum = "Lav_PANNING_STRATEGIES", doc = """
The panning strategy for any source configured to delegate to the environment.
All new sources delegate to the environment by default.

If you want to change this property for a specific source, set {{propref("Lav_OBJTYPE_SOURCE_NODE", "Lav_SOURCE_CONTROL_PANNING")}} to true.
"""),
    property(name = "output_channels", identifier = "Lav_ENVIRONMENT_OUTPUT_CHANNELS", type = p_int, range = (0, 8), default = 2, doc = """
Environments are not smart enough to determine the number of channels their output needs to have.
If you are using something greater than stereo, i.e. 5.1, you need to change this property.
The specific issue solved by this property is the case in which one source is set to something different than all others, or where the app changes the panning strategies of sources after creation.

Values besides 2, 4, 6, or 8 do not usually have much meaning.
"""),
    property(name = "reverb_distance", identifier = "Lav_ENVIRONMENT_REVERB_DISTANCE", type = p_float, range = (0.0, inf), default = 75.0, doc = """
The distance at which a source will be heard only in the reverb.

See documentation on the {{"Lav_OBJTYPE_SOURCE_NODE"|node}} for a specific explanation.
By default, sources get the value of this property from the environment.
To control this property on a per-source basis, set {{propref("Lav_OBJTYPE_SOURCE_NODE", "Lav_SOURCE_CONTROL_REVERB")}} to true.
"""),
    property(name = "min_reverb_level", identifier = "Lav_ENVIRONMENT_MIN_REVERB_LEVEL", type = p_float, range = (0.0, 1.0), default = 0.15, doc = """
The minimum reverb level allowed.

If a send is configured to be a reverb send, this is the minimum amount of audio that will be diverted to it.

Behavior is undefined if this property is ever greater than the value of {{propref("Lav_OBJTYPE_ENVIRONMENT_NODE", "Lav_ENVIRONMENT_MAX_REVERB_LEVEL")}}.

By default, sources look to their environment for the value of this property.
If you wish to set it on a per-source basis, set {{propref("Lav_OBJTYPE_SOURCE_NODE", "Lav_SOURCE_CONTROL_REVERB")}} to true.
"""),
    property(name = "max_reverb_level", identifier = "Lav_ENVIRONMENT_MAX_REVERB_LEVEL", type = p_float, range = (0.0, 1.0), default = 0.6, doc = """
The maximum amount of audio to be diverted to reverb sends, if any.

Behavior is undefined if this property is ever less than {{propref("Lav_OBJTYPE_ENVIRONMENT_NODE", "Lav_ENVIRONMENT_MIN_REVERB_LEVEL")}}.

By default, sources look to their environmlent for the value of this property.
If you wish to set it on a per-source basis, set {{propref("Lav_OBJTYPE_SOURCE_NODE", "Lav_SOURCE_CONTROL_REVERB")}} to true.
"""),
    extra_function(function = "Lav_environmentNodePlayAsync", doc = """
Play a buffer, using the specified position and the currently set defaults on the world for distance model and panning strategy.
This is the same as creating a buffer and a source, but Libaudioverse retains control of these objects and caches them internally to avoid allocations.
When the buffer finishes playing, the source is automatically disposed of.
""", param_docs = {
        "bufferHandle": "The buffer to play",
        "x": "The x-component of the position.",
        "y": "The y-component of the position.",
        "z": "The z-component of the position",
        "isDry": "If true, we avoid sending to the effect sends.",
    }, defaults = {
        "x": 0.0,
        "y": 0.0,
        "z": 0.0,
        "isDry": False,
    }),
output(channel_type = dynamic, channels = r"""Depends on {{propref("Lav_OBJTYPE_ENVIRONMENT_NODE", "Lav_ENVIRONMENT__OUTPUT_CHANNELS")}}.""",
doc = r"""The aggregate signal from all sources.""")])
