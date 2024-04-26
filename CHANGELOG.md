# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [unreleased]

### Fixed

- doc: Directory containing USB-2.0 camera firmware for firmware updates now points to correct dir.

## [1.1.1] - 2023.04.26

### Fixed

- Examples are now part of the dev package
- Device lost handling could cause segfaults under certain circumstances

## [1.1.0] - 2023.03.31

### Added

Devices:
- Support for 52U usb-2.0 cameras
- Support for new DFG/HDMI properties

UDev:
- rule to disable usb power management for TIS devices

tcammainsrc:
- own bufferpool
- io-mode property, currently consists out of user-ptr and mmap
- simple virtual camera, set `TCAM_VIRTCAM_DEVICES=name0:name1` to generate devices
- Mark GstBuffer with GST_BUFFER_FLAG_CORRUPTED when image is damaged.
  Requires drop-incomplete-buffer=false

tcambin:
- GstChildProxy interface

tcamsrc:
- GstChildProxy interface

tcam-capture:
- Ability to save images
- Ability to save videos

tcam-gigetool:
- check-control command
  Get IP/port that controls the camera

dependency-manager:
- option --ignore for install routine
  Allows preventing dependencies from being installed
- Warning if 'lsb_release' can not be found
- dependency description for Debian 10 (Buster)
- dependency description for Ubuntu 22.10

packaging:
- split development dependencies into own package

### Changed

CMake:
- aravis is cloned into the build directory, not into the source tree
- meson is cloned into the build directory, not into the source tree

### Fixed

tcammainsrc:
- Locked properties now return TCAM_ERROR_PROPERTY_NOT_WRITEABLE when they are locked and being set.
  Previously they could still be set even when locked.

tcam-capture:
- Property JSON description was not updated when re-opening the dialog
- No more critical warning if camera has no trigger mode

tcam-ctrl:
- Fix help text for `--load` and `--save`

tcam-gigetool:
- Fixed misidentification of camera identifier

aravis:
- TriggerMode is now respected when setting formats

cmake:
- TCAM_BUILD_UVC_EXTENSION_LOADER_ONLY build

installation:
- tcamgststatistics.so is now installed into `TCAM_INSTALL_LIB`

doc:
- TCAM_DOWNLOAD_MESON now has correct default value (ON)

examples:
- 10-metadata.py - example should now load tcamgststatistics.so even when installed
- 02-set-properties.c - pipeline was not freed correctly
- 02-set-properties.py - Catch exception when setting properties
- 06-softwaretrigger.py - Catch exception when setting softwaretrigger

packaging:
- Removed dependencies that are not required for runtime

## [1.0.0] - 2022.06.01

### Added

General:
- internal dependency to fmt
- environment variable `TCAM_ARV_STREAM_OPTIONS`, allows setting of ArvStream options
- environment variable `TCAM_GIGE_HEARTBEAT_MS`, allows setting the heartbeat timeout
- tiscamera threads now have names for easier identification during debugging.

CMake:
- clang-format definition, call `make clang-format` for formatting
- cmake flag `TCAM_DOWNLOAD_MESON` for legacy systems to allow modern aravis installations.
  for the referenced aravis version, meson-version=0.56 is necessary
- Module configuration string to log and version info

GStreamer:
- GstTcamDeviceProvider, see documentation for usage.
- tcamconvert gstreamer conversion element
  replaces tcamwhitebalance and provides color-conversion services.
- tcambin property `conversion-element`
  Used to select the internal conversion elements.
  Available options are `tcamconvert`, `tcamdutils`, `tcamdutils-cuda` and `auto`.
- tcambin property `available-caps` to fetch available device caps
- tcambin/tcamsrc property `tcam-device` to directly set a GstDevice instance to open
- tcambin/tcamsrc property `tcam-properties`. GstStructure like description of properties.
  See documentation for more details.
- tcambin/tcamsrc signals `device-open`/`device-close`
- support for tcamdutils-cuda
- example on how to retrieve GstMeta data in python
- GstQuery for AcceptCaps and Caps in tcammainsrc

Tools:
- `tcam-ctrl --packages` - list all installed 'The Imaging Source' packages and their version
- `tcam-ctrl --version` now lists configured modules

### Changed

General:
- property API changed to tcam-property-1.0 (part of tiscamera)
- device list API changed to GstDeviceProvider
- aravis library minimum version is now 0.8.20.
- logging now uses spdlog and is enabled via e.g. `GST_DEBUG=tcam*:3`
- Filter for DFG/USB2pro. Device will no longer appear in device listings.

CMake:
- output directory for binaries/libraries to <build-dir>/bin and <build-dir>/lib
- installation variables are now all defined in CMakeInstall.cmake
- cmake option `TCAM_BUILD_ARAVIS` now defaults to `ON`
- cmake option `TCAM_BUILD_DOCUMENTATION` now defaults to `ON`
- cmake option `TCAM_ARAVIS_USB_VISION` now defaults to `ON`

GStreamer:
- gstreamer buffers are now marked as "live"
- tcamsrc/tcammainsrc 'do-timestamp' default changed to false.
- tcambin/tcamsrc/tcammainsrc must be in `GST_STATE_READY` or higher before you can access a device
- tcambin/tcamsrc `property-state` renamed to `tcam-properties-json`
- tcambin version tcamdutils does not check for parity anymore,
  but for minimum version to ensure compatability.
- tcamautoexposure gstreamer element - functionality now in tcammainsrc
- tcamwhitebalance gstreamer element - functionality now in tcammainsrc,
  application in tcamconvert/tcamdutils
- tcamautofocus gstreamer element - functionality now in tcammainsrc
- gstreamer resolution ranges now contain a step size
- polarization caps have been renamed, check caps descriptions for new ones

Tools:
- renamed gige-daemon to tcam-gige-daemon for consistency.
- tcam-gigetool rewrite. Now implemented with C++.
  Output has changed.
- tcam-gige-daemon maximum for devices increased to 50.
- tcam-capture rewrite. Now implemented with C++.
  Aimed to be as simple as possible.
  For more complex use cases use tis-measure.
  https://www.theimagingsource.com/support/downloads-for-linux/end-user-software/icmeasureappimage/

### Fixed

- "device-lost" notifications could take over 3 minutes for aravis cameras.
  Should now appear within 5 seconds.
- generated debian packages are now installable when no systemd exists.
  This is only relevant when compiling with aravis.

### Removed

- Support for 23G, 23U, 73U and AFU130
- gstreamer elements tcamautoexposure, tcamwhitebalance, tcamautofocus
- tcamprop 0.1 API
- firmware-update
  Moved to a separate repository: https://github.com/theimagingsource/tcam-firmware-update
- camera-ip-conf
  Use tcam-gigetool instead.
- tcambin property `use-dutils`
  Use tcambin property `conversion-element` instead.
- environment variable TCAM_ARV_PACKET_REQUEST_RATIO, replaced with TCAM_ARV_STREAM_OPTIONS
- environment variable TCAM_LOG, replaced with gstreamer logging category `tcam-libtcam`
- unmaintained unit tests
- TCAM_LOG environment variable.
  Use GST_DEBUG category `tcam-libtcam` instead.
- Legacy uvc extension descriptions for uvcdynctrl

### Known Issues

- In camera Auto Functions ROI may not correctly lock properties
- In camera Auto Functions ROI Top/Left/Width/Height may not correctly update when using preset
- AFU050: image/jpeg,width=1280,height=960 may cause segfault when used.
  Cause is in external library.
- GstQueryCaps for Usb3Vision devices via aravis does not work correctly

## [0.14.0] - 2021.07.05

### Added

- 'tegra' device type
- support for tcamtegrasrc
- PWL format support

### Changed

- Reworked examples to be more applicable to all cameras.
- property state description is now applied when going PAUSE->PLAYING
- Device lost handling for v4l2 cameras when waiting for images.
  Timeout now causes a warning. Device lost will not be triggered!
- examples now have a default gstreamer log level of WARNING
- tcambin writes a warning message to the GstBus when a version
  incompatability with tcamdutils is detected
- New project logo

### Fixed

- Using only the `type` property of tcambin/tcamsrc now works
- Added missing max resolution for DxK AFU420 cameras

## [0.13.1] - 2021.02.05

### Changed

- tcambin applies state descriptions when changing to GST_STATE_PLAYING
- debian package name for master branch now contains commit count
- debian package name now contains distribution release to prevent
  confusion about installation problems due to dependencies

### Fixed

- tcam-capture display bug for bools
- tcambin caps negotiations for GRAY16_LE
- multiple memory leaks
- deadlock in tcammainsrc
- device lost no longer fires when using long exposure times
- tcam-ctrl did not correctly verify <serial>-<type> combos

## [0.13.0] - 2020-11-23

### Added

- firmware files
  dfk72uc02_140.euvc
  dmk72uc02_140.euvc
  dfk72uc02_af_146.euvc
  dmk72uc02_af_146.euvc
- tcam-uvc-extension-loader can now be built/installed without other libraries.
  Set cmake option TCAM_BUILD_UVC_EXTENSION_LOADER_ONLY to ON to activate.
- camera-ip-conf can now be built/installed without other libraries.
  Set cmake option TCAM_BUILD_CAMERA_IP_CONF_ONLY to ON to activate.
- firmware-update can now be built/installed without other libraries.
  Set cmake option TCAM_BUILD_FIRMWARE_UPDATE_ONLY to ON to activate.
- tcammainsrc - Replaces tcamsrc in functionality.
- 'pimipi' device type
- 11-device-state example
- CMake switch `TCAM_BUILD_NO_GUI` to disable all gui build targets and dependency inclusions.
  See documenation for further details.
- script dependency-manager as a way to improve dependency handling for users/packaging
- conflicts, provides, replaces fields in Debian package description for `tiscamera-tcamprop`
- dependency-manager - replacement for install-dependencies.sh
  offers wider range of functionality and allows addressal of different distributions
  See documentation for further details
- C_INCLUDE_PATH to env.sh

### Changed

- cmake user options are now all defined in a separate file
- tcam-ctrl --load now accepts files and attempts to load them as json
- gstreamer elements now have the same version string that is used for releases
- tcamsrc is now a wrapper around all potential TIS source elements
- tiscamera and tiscamera-dutils are now version locked. tcambin will log a warning
  when a version mismatch is detected. Use 'use-dutils=true' to overwrite.
  Mismatching version are not supported.
- All gstreamer elements now only use tcamprop for property interactions
- tcam-ctrl now uses tcamsrc for all device interactions
- tcam-capture now uses xvimagesink for display purposes
- All uses of CMAKE_SOURCE_DIR and CMAKE_BINARY_DIR have been replaced by
  internal variables
- Renamed folder for third party libraries from `dependencies` to `external`
- Moved dependency descriptions to subfolder `dependencies`
- Examples now have correct memory handling
- 'Exposure Time (us)' now has category 'Exposure'

### Fixed

- dependency description allows usage of libzip4 and libzip5
- gst: Apply bin state when all elements are verified playing
- tcambin: ensure serial and type are always defined
- camera-ip-conf can now correctly write newer firmware versions
- package generation now works under Ubuntu 20.04
- camera-ip-conf/tcam-gigetool now support newer
  firmware files that have correct access control
- Json property descriptions can now be used to set 'button' type properties
- Various memory leaks
- gige-daemon now correctly deletes lock file
- `gige-daemon start --no-fork` crash when stopping
- serial-type combinations where not always correctly identified.
  Now `Aravis` and other capitalized strings are converted to lowercase before being
  interpreted.
- tcamsrc (now tcammainsrc) num-buffers delivered buffer where one short
- camera-ip-conf was unable to set name without ip/netmask/gateway
- various documentation errors

### Removed

- firmware files
  dfk72uc02_129.euvc
  dfk72uc02_158.euvc
  dmk72uc02_129.euvc
  dmk72uc02_146.euvc
  dmk72uc02_158.euvc
  dmk72uc02_AF_140.euvc
  dmk72uc02_af_144.euvc
  dfk72uc02_AF_140.euvc
- tcambiteater - functionality is no part of tcamdutils element
- tcam-capture lost the following features:
  - zoom
  - pixel color under mouse
  - ROI selection/display via overlay
  - fit-to-view
- tcammainsrc/tcamsrc property `camera`. Use tcamprop instead.

### Known Issues

- Device lost for GigE cameras can be extremely slow.
- Documentation: RemovedInSphinx40Warning: The app.add_javascript() is deprecated.
                 Please use app.add_js_file() instead. app.add_javascript(path)
        Not fixed due to sphinx version incompatability on older Ubuntu versions.
- tcam-capture may experience a thread lock when changing format on some systems.
  Reliable reproducability was not possible during testing.

## [0.12.0] - 2020-05-27

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
  Allows usage of USB3Vision via aravis.
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
- tcamsrc now answers latency queries
- example for gstreamer metadata retrieval
- tcamprop function tcam_prop_get_device_serials_backend
  Retrieves a list of all connected device serial numbers.
  Appended to the serial number is the backend, separated by a hyphen "-".

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
- tcam-capture fps display only considers the last 5 seconds
- aravis version to HEAD of aravis-0.6 branch
- gige-daemon now queries aravis directly

### Fixed

- Compiler warnings
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
- erroneous dependency description in
  generated .deb files when working with -DBUILD_TOOLS=OFF
- Wrong property mapping for GPIO for usb33/37 cameras (Now GPIn)

### Removed

- unused header files
- tcam-ctrl -s flag for setting formats.
- tcam-ctrl -s flag for setting properties. Replaced with state system.
- unit tests for tiscamera-dutils. They are now part of the tiscamera-dutils repository

### Known Issues

- On some systems the python3 module installation path is set wrong.
- tcam-capture sometimes has problems with streaming when handling Usb3Vision cameras.


## [0.11.1] - 2019-05-22

### Fixed

- Memory leak in the udev device iteration
- Installation issues for tcam-gigetool with custom CMAKE_INSTALL_PREFIX
- Installation issues for tcam-capture with custim CMAKE_INSTALL_PREFIX
- Issues with UVC extension units for USB 23 and 33 cameras concerning 'Trigger Polarity'

## [0.11.0] - 2019-05-13

### Deprecated

- XML uvc extension descriptions are no longer maintained.
  To be removed in future release.

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

## [0.9.1] - 2017.09.15

## [0.9.0] - 2017.07.27

## [0.8.2] - 2017.06.28
