#####
Flags
#####

Properties can have the following flags:

TCAM_PROPERTY_FLAG_DISABLED
---------------------------
Bit: 0x0001

This control is permanently disabled and should be ignored by the application.
Any attempt to change the control will result in an EINVAL error code.

TCAM_PROPERTY_FLAG_GRABBED
--------------------------
Bit: 0x0002

This control is temporarily unchangeable, for example because another
application took over control of the respective resource. Such controls may be
displayed specially in a user interface. Attempts to change the control may
result in an EBUSY error code.

TCAM_PROPERTY_FLAG_READ_ONLY
----------------------------
Bit: 0x0004

This control is permanently readable only.

TCAM_PROPERTY_FLAG_EXTERNAL
---------------------------
Bit: 0x0008

This control is realized through library code and is not available in the camera

TCAM_PROPERTY_FLAG_INACTIVE
---------------------------
Bit: 0x0010

This control is not applicable to the current configuration and should be
displayed accordingly in a user interface. For example whitebalance may be
disabled when retrieving a greyscale image stream.

TCAM_PROPERTY_FLAG_WRITE_ONLY
-----------------------------
Bit: 0x0020

This control is permanently writable only. Any attempt to read the control will
result in an EACCES error code error code. This flag is typically present for
relative controls or action controls where writing a value will cause the device
to carry out a given action (e. g. motor control) but no meaningful value can be
returned.

TCAM_PROPERTY_FLAG_IS_LOGARITHMIC
---------------------------------
Bit: 0x0040

This control should be displayed using a logarithmic scale as most actions will
take place in a small part of the overall range.

TCAM_PROPERTY_FLAG_REQUIRES_RESTART
-----------------------------------
Bit: 0x0080

This control requires a restart of the stream to be applied.
