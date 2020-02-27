# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Missing Strobe Enable and Strobe Polarity for USB-2.0 cameras
- tcamsrc property 'camera-buffers'
  Allows modification of the number of buffers the backend uses.
- Missing IMX Low-Latency Mode for USB-3 33, 37, 38 cameras
- Property mappings for
  IMX Low-Latency Mode
  Trigger Global Reset Release
- tcam-capture: Double click on sliders resets to default values
- tcam-capture: SpinBox buttons do not randomly stop accepting input
- install-dependencies.sh updates the package cache before installation
  To skip this step, use `--no-update`
- tcamautoexposure support for GRAY16_LE
- Installation of examples folder in $PREFIX/share/theimagingsource/tiscamera
- Firmware v3024 for dmk/dfk 22uc03 auto focus cameras
- tiscamera-env.sh script to source installed tiscamera instances
- cmake option TCAM_ARAVIS_USB_VISION
  Allows usage of USB3Vision via aravis. gige-daemon has to be disabled.
  v4l2 will automatically disabled.
- uvc extension unit for 37U cameras.
- property state system
  tcamsrc and tcambin have the property 'state' containing a JSON string describing
  the current property/value list.
- type property for tcamsrc and tcambin
  Allows the selection of the backend when multiple are available
- `tcam-ctrl --load` to load property state string
- `tcam-ctrl --save` to save property state string
- caps definitions for polarization cameras
- TCAM_ARV_PACKET_REQUEST_RATIO environment variable
- aravis version to HEAD of aravis-0.6 branch

### Changed

- Reference system is now Ubuntu 18.04 LTS

- Installation of static data is now unified under /usr/share/theimagingsource/tiscamera/
- tcambin now only initializes jpegdec when tcamsrc offers image/jpeg
- "Override Scanning Mode" category is now "Partial Scan"
- "Reverse X" and "Reverse Y" category is now "Image"
- Moved properties ReverseX and ReverseY to category 'Image'
- GRAY8 is now preferred over GRAY16_LE in caps negotiation
- tcamsrc num-buffers default is now -1. This is now identical to v4l2src.
- Log output now only contains the filename and not the absolute path.
- logging timestamp is now YYYY-mm-ddTHH:MM:SS:MS
- tcam-capture: slider with a range larger than 5000 now have a logarithmic behavior
- tcam-capture: Number Text Boxes do not update while user is editing
- cmake option -DBUILD_TOOLS now defaults to ON
- udev rules now only contain extension loading when required by BUILD_V4L2.
- tcam-ctrl argument handling has been reworked. This changes the help message.
- udev rules file now only contains uvc extension loading when v4l2 is configured
- Line endings in 33U firmware files are handled differently

### Fixed

- Compiler warning
- Loading of uvc extension units for USB-2.0 cameras
- Installation path with tcam_capture module
- Threading issues with GstBus messages
- Multiple issues with python examples
- Issues with GigE camera bool handling
- Issues with python examples
- gsttcamautoexposure iris max is now handled correctly
- cmake install directories could not be set by user
- tcam-capture issue with jumping ROI overlay
- 'Strobe Exposure' control in extension units for 33U and 37U
- gstmetatcamstatistics.h is installed into gstreamer-1.0 include directory
- env.sh now respects existing environment variables
- Segfault on buffer destruction on stream end in gsttcamsrc
- Pipelines sometimes did not end when backend device could not be opened
- Fix min/max ranges for tcamautoexposure properties with no device set
- tcam-capture: Double slider behavior is now correct
- thread lock up in the V4l2Device notification thread during destruction
- Segfault in device lost callback in gsttcamsrc

### Removed

- unused header files
- tcam-ctrl -s flag for setting formats.
- tcam-ctrl -s flag for setting properties. Replaced with state system.
- unit tests for tiscamera-dutils. They are now part of the tiscamera-dutils repository


## [0.11.1] - 2019-05-22

### Fixed

- Memory leak in the udev device iteration
- Installation issues for tcam-gigetool with custom CMAKE_INSTALL_PREFIX
- Installation issues for tcam-capture with custim CMAKE_INSTALL_PREFIX
- Issues with UVC extension units for USB 23 and 33 cameras concerning 'Trigger Polarity'

## [0.11.0] - 2019-05-13

### Added

- tcam-capture now has a --fullscreen flag to make it start in fullscreen mode
- tcam-capture now has a fit-to-view button to resize the display
- tcam-capture now has a ROI display and selection capabilities
- tcam-capture now has configurable global keybindings allowing for
  fullscreen, image saving, triggering and opening the device dialog
- Generation of user documentation. Enable with -DBUILD_DOCUMENATION=ON
- Auto Iris functionality to gsttcamautoexposure element
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
- 12-Mono Support for GigE devices
- env.sh to add build directory to current environment
- install-dependencies.sh to automatically install dependencies
- Filter for genicam properties SensorPixelHeight and SensorPixelWidth
- TAG+="uaccess", TAG+="udev-acl to USB device permissions

### Changed

- lost-device event will be triggered more aggressively
- The aravis backend will now always attempt to use a realtime thread
- The internal aravis is built with fast-heartbeat=ON
- The caps output of 'tcam-ctrl -c' does not contain type descriptions.
  This means the descriptions can now be copy pasted for gst-launch
- internal aravis version to 0.6.2
- Switch to different auto-exposure algorithm
- gstreamer elements that interact with tcamsrc elements now search upstream
- Udev rules
  -- Changed discovery of legacy cameras
  -- Changed TAGS/mod to be aravis USB3Vision compatible
- TcamProp properties are now available when in GST_STATE_READY
- aravis is now a cmake external project and not a git submodule
- USB device mode is now 0666
- legacy usb cameras are registered differently. This should minimize problems with udev.

### Removed

- unused camera-ip-conf gui
- uvcdynctrl dependency
- Most examples. Now only API examples exist.
  Complex examples were move to https://github.com/TheImagingSource/Linux-tiscamera-Programming-Samples

### Fixed

- The gige-daemon showed connection problems to clients on some systems due to permissions.
- compilation error due to missing header on some systems due to POSIX compatability issues.
- tcamsrc sometimes generated double resolution entries for ranges.
- tcam-capture --serial was not respected
- Segmentation faults when program exits
- tcam-gigetool installation problems
- Faulty serial number identification for GigE cameras when the gige-daemon was not running

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
