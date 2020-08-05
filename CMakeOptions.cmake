
# Copyright 2020 The Imaging Source Europe GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# This file contains all options/variables that users
# can set to configure tiscamera


option(BUILD_GST_1_0  "Build gstreamer-1.0 plugins?"         ON)
option(BUILD_ARAVIS   "Include GigE support via aravis"      OFF)
option(BUILD_V4L2     "Include support for USB cameras"      ON )
option(BUILD_LIBUSB   "Include support for LibUsb cameras"   ON )
option(BUILD_TOOLS    "Build additional utilities"           ON)
option(BUILD_DOCUMENTATION "Build internal code documentation"    OFF)
option(BUILD_TESTS    "Build tests."                         OFF)


option(TCAM_INTERNAL_ARAVIS "Use internal aravis dependency instead of system libraries" OFF)
option(TCAM_ARAVIS_USB_VISION "Use aravis usb vision backend. Disables v4l2." OFF)

if (TCAM_ARAVIS_USB_VISION)
  # activate aravis since we need it for usb3vision
  set(BUILD_ARAVIS ON CACHE BOOL "Include GigE support via aravis" FORCE)

endif (TCAM_ARAVIS_USB_VISION)

option(TCAM_BUILD_UVC_EXTENSION_LOADER_ONLY "Disable all other libraries/tool and only build tcam-uvc-extension-loader." OFF)

option(TCAM_ENABLE_DATA_INSTALL "Install static data from data/examples folder." ON)
option(TCAM_ENABLE_BASE_LIBRARIES "Build/install base libraries." ON)

# disable all other things
# we only want the extension loader and associated things
if (TCAM_BUILD_UVC_EXTENSION_LOADER_ONLY)

  set(BUILD_GST_1_0 OFF)
  set(BUILD_ARAVIS OFF)
  set(BUILD_V4L2 OFF)
  set(BUILD_LIBUSB OFF)
  set(BUILD_TOOLS OFF)
  set(BUILD_DOCUMENTATION OFF)
  set(BUILD_TESTS OFF)
  set(TCAM_ARAVIS_USB_VISION OFF)

endif (TCAM_BUILD_UVC_EXTENSION_LOADER_ONLY)
