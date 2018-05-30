# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

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


### Removed

- TCAM_PROPERTY_AUTO_REFERENCE - no handled by TCAM_PROPERTY_AUTO_EXPOSURE
- installation of internal header files

## [0.9.1]
