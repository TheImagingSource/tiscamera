# The Imaging Source Linux Repository

This folder contains examples for using the tiscamera software in various
programming languages and/or different applications. The same examples and file
structure is used for all programming examples. Please note that not all
examples exist for every language.

Supported programming languages: C, C++, Python, LUA.

## Programming Examples

list_devices:

    Shows how to get a list of all supported devices currently connected to
    the computer.

list_formats:

    Prints out a list of all video formwats supported by a device.

list_properties:

    Prompts for a video device and prints out a list of all supported
    properties as well as their current values and other details.

live_video:

    Shows a live video stream from the device using the GTK+3 libraries.
    This example also shows how to save video images to a JPEG file.

property_dialog:

    Extends the live_video example with a property dialog.

## Other Examples

ROS:

    This folder contains an interface to the Robot Operating System (ROS).
    ROS is available for download on http://www.ros.org

## Licensing

All files are published under the Apache License 2.0.

