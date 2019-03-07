# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- tcam-capture now has a --fullscreen flag to make it start in fullscreen mode
- tcam-capture now has a fit-to-view button to resize the display
- tcam-capture now has a ROI display and selection capabilities
- tcam-capture now has configurable global keybindings allowing for
  fullscreen, image saving, triggering and opening the device dialog
- Generation of user documentation. Enabled with -DBUILD_USER_DOC=ON
- Auto Iris functionality to gsttcamautoexposure element
- bash auto completion for tcam-ctrl
- bash auto completion for tcam-capture
- bash auto completion for gige-daemon
- AFU050 will have much quicker device lost notification
- tcamsrc now adds a GstMeta object to each buffer to transport additional information
- Add property drop-incomplete-buffer to tcamsrc
- TCAM_GIGE_PACKET_SIZE environment variable
- Add gain auto lower limit to usb3 uvc extension unit
- Some basic unit/integration tests.
  Enabled with -DBUILD_TESTS=ON.
  Run with 'make test'.
- tcam-uvc-extension-loader as a replacement for uvcdynctrl
- libuuid dependency


### Changed

- lost-device event will be triggered more aggressively
- The aravis backend will now always attempt to use a realtime thread
- The internal aravis is built with fast-heartbeat=ON
- The caps output of 'tcam-ctrl -c' does not contain type descriptions.
  This means the descriptions can now be copy pasted for gst-launch
- internal aravis version to 0.6.1
- Switch to different auto-exposure algorithm
- gstreamer elements that interact with tcamsrc elements now search upstream

### Removed

- unused camera-ip-conf gui
- uvcdynctrl dependency

### Fixed

- The gige-daemon showed connection problems to clients on some systems due to permissions.
- compilation error due to missing header on some systems due to POSIX compatability issues.
- tcamsrc sometimes generated double resolution entries for ranges.
- tcam-capture --serial was not respected
- Segmentation faults when program exits

## [0.10.0] - 2018-07-31

### Added

- Add CHANGELOG.md
- Support for the AFU420
- Support for the AFU050
- Install routines for tcam-capture
- Add flags TCAM_PROPERTY_FLAG_IS_LOGARITHMIC and TCAM_PROPERTY_FLAG_REQUIRES_RESTART
- gsttcambin now sends a message on the bus "Working with src caps: %s" about the caps that
  are selected for the source
- gsttcamsrc now forwards error messages and warnings from the backends to the
  gstreamer log and bus
- gsttcamsrc now send a device-lost message on the gst bus when the device is unusable
- basic doxygen documentation
- Add log level 'TRACE'
- make package command to create a deb package
- Add support for gsttcamdutils modules
- Add support for non 8-bit bayer formats via gsttcamdutils
- Add gsttcambiteater module to reduce BGR64 to BGR32
- Add exposure-min and gain-min properties to gsttcamautoexposure
- Add device-caps property to tcambin to set caps for internal tcamsrc
- Add support in firmware-update for 33u and 37U cameras
- Add property category "Color Matrix"

### Changed

- Actually make tcam-capture usable
- Added property flag is_logarithmic
- Added property flag requires_restart
- Make gsttcamsrc propagate error and warnings to gstreamer log and bus
- GstBuffer memory is now asynchroniously given back to the backend, meaning
  downstream elements can hold the buffer indefinitely
- Aravis backend now sets packet resent to true
- Aravis backend now tries to automatically determine the gv packet size
- backend libraries moved to subfolders
- Increased minimum cmake version to 3.2
- Properties that occur multiple time due to firmware behavior are now filtered
  in the v4l2 backend
- gsttcamwhitebalance is now multi-threaded to increase performance for large
  images
- gsttcamautofocus properties now list their category as "Lens"
- 'tcam-ctrl -c' now uses gst_caps_to_string internally instead of creating a
  description itself
- Default installation prefix now is /usr/ and not /usr/local
- Changed cmake installation variables types to PATH

### Fixed

- Segfault when DeviceIndex::get_device_list was called and gige-daemon was not running
- gsttcamautoexposure now correctly handles gain/exposure ranges
- gsttcamwhitebalance now correctly handles bggr
- gsttcamwhitebalance tcam_prop interface now returns the correct values for
  whitebalance-auto and camera-whitebalance
- gsttcamautofocus ROI now behaves correctly
- gsttcamautofocus "Auto Focus One Push" has to be triggered only once to
  actually do something
- tcam-ctrl set now handles all property cases
- Logger instance now works across library boundaries
- removed compile flag that hid warnings, warnings have been fixed

### Removed

- TCAM_PROPERTY_AUTO_REFERENCE - no handled by TCAM_PROPERTY_AUTO_EXPOSURE
- installation of internal header files

## [0.9.1]
