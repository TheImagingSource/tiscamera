
# Design

This document describes the general design of the tiscamera project.

The libraries use C++ to allow an easy integration of existing C++ code.

## Backends

A backend is defined as a wrapper around a system/third-party API that converts
between the API camera specification and the tiscamera camera definition.

Each backend is in a separate shared library to enable independent updates and
easy packaging.

The API a backend has to implement is defined in the file 'devicelibrary.h'.

The interaction between camera object and backend is done through the abstract
DeviceInterface class.

## Algorithms

Our algorithms can be found in a separate library. This library can be used
independently from the rest of our project.

## Gstreamer

For simple multimedia handling gstreamer was chosen as a general purpose
framework.
An interaction without gstreamer is not planned.
The provided modules consist out of:

- tcamsrc
  General purpose camera source.
- interpretation plugins
  Analyzing the images and correcting the camera settings. e.g. autoexposure, autofocus
- adjustment plugins
  Adjusting the content of the image. e.g. white-balance
- tcambin
  Automatic management of all of the aforementioned elements.

For more information read the gstreamer page (TODO: link)

## GObject Introspection

TODO: API documentation
For simple interaction with camera properties that are not reachable through
gstreamer, a gi-interface is provided.

## Tools

We offer various tools to help users with their cameras.

These tools either act as standalone tools to act as a fallback or they rely on
our main library and offer an easy camera representation/interaction.

### Tools that act independently

camera-ip-conf
firmware-update

### Tools that use our library

tcam-ctrl - commandline tool for camera interaction
gige-daemon - simple daemon for GigE camera lookup
