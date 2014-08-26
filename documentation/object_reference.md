#Libaudioverse Object Reference#

This page is an overview of all Libaudioverse objects.
The following tables completely describe all implemented Libaudioverse objects in terms of the C API.

For binding-specific references, see the bindings themselves.
Python uses docstrings, for example.
Also see the Binding-specific Notes section of this documentation.

Usually, the names of objects can be inferred by mentally stripping the `Lav_OBJTYPE_` prefix from the identifying enumerant and looking for `suffix_object` or `SuffixObject` depending on the conventions of your language.
These transformations are done for you in this document, listed as Camelcase Identifier and underscore identifier, respectively.

For a discussion of property types, see Property Types.  For a discussion of callbacksk see Callbacks.  This document is focused only on the objects themselves.

##Generic Properties Common to All Objects##

###Properties###

Name | C Constant | Type | Range | Default | Description
-----|-----|-----|-----|-----
suspended | Lav_OBJECT_SUSPENDED | boolean |  | 0 | None Defined


##Amplitude Panner##

###Properties###

Name | C Constant | Type | Range | Default | Description
-----|-----|-----|-----|-----
azimuth | Lav_PANNER_AZIMUTH | float | ['-INFINITY', 'INFINITY'] | 0.0 | None Defined
elevation | Lav_PANNER_ELEVATION | float | [-90.0, 90.0] | 0.0 | None Defined
channel_map | Lav_PANNER_CHANNEL_MAP | float_array |  | [-90, 90] | None Defined


##Attenuator##

###Properties###

Name | C Constant | Type | Range | Default | Description
-----|-----|-----|-----|-----
multiplier | Lav_ATTENUATOR_MULTIPLIER | float | [0.0, 'INFINITY'] | 1.0 | None Defined


##Delay Line##

###Properties###

Name | C Constant | Type | Range | Default | Description
-----|-----|-----|-----|-----
feedback | Lav_DELAY_FEEDBACK | float | [0.0, 1.0] | 0.0 | None Defined
delay_max | Lav_DELAY_DELAY_MAX | float | [0.5, 5.0] | 1.0 | None Defined
delay | Lav_DELAY_DELAY | float | [0.0, 0.0] | 0.001 | None Defined
interpolation_time | Lav_DELAY_INTERPOLATION_TIME | float | [0.001, 'INFINITY'] | 0.001 | None Defined


##File##

###Properties###

Name | C Constant | Type | Range | Default | Description
-----|-----|-----|-----|-----
looping | Lav_FILE_LOOPING | boolean |  | 0 | None Defined
position | Lav_FILE_POSITION | double | [0.0, 0.0] | 0.0 | None Defined
pitch_bend | Lav_FILE_PITCH_BEND | float | [0, 'INFINITY'] | 1.0 | None Defined

##callbacks##
Name | C Constant | description
-----|-----
end | Lav_FILE_END_CALLBACK | None Defined

##Hrtf##

###Properties###

Name | C Constant | Type | Range | Default | Description
-----|-----|-----|-----|-----
azimuth | Lav_PANNER_AZIMUTH | float | ['-INFINITY', 'INFINITY'] | 0.0 | None Defined
elevation | Lav_PANNER_ELEVATION | float | [-90.0, 90.0] | 0.0 | None Defined


##Mixer##

###Properties###

Name | C Constant | Type | Range | Default | Description
-----|-----|-----|-----|-----
max_parents | Lav_MIXER_MAX_PARENTS | int | [0, 'MAX_INT'] | 0 | None Defined
inputs_per_parent | Lav_MIXER_INPUTS_PER_PARENT | int | [0, 0] | 0 | None Defined


##Simple Source##

###Properties###

Name | C Constant | Type | Range | Default | Description
-----|-----|-----|-----|-----
orientation | Lav_3D_ORIENTATION | float6 |  | [0.0, 0.0, -1.0, 0.0, 1.0, 0.0] | None Defined
position | Lav_3D_POSITION | float3 |  | [0.0, 0.0, 0.0] | None Defined
distance_model | Lav_SOURCE_DISTANCE_MODEL | int | [0, 0] | 0 | None Defined
max_distance | Lav_SOURCE_MAX_DISTANCE | float | [0.0, 'INFINITY'] | 50.0 | None Defined


##Sine##

###Properties###

Name | C Constant | Type | Range | Default | Description
-----|-----|-----|-----|-----
frequency | Lav_SINE_FREQUENCY | float | [0, 'INFINITY'] | 440.0 | None Defined


##World##

###Properties###

Name | C Constant | Type | Range | Default | Description
-----|-----|-----|-----|-----
orientation | Lav_3D_ORIENTATION | float6 |  | [0.0, 0.0, -1.0, 0.0, 1.0, 0.0] | None Defined
position | Lav_3D_POSITION | float3 |  | [0.0, 0.0, 0.0] | None Defined


