
# Copyright 2017 The Imaging Source Europe GmbH
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

set(lib_libusb
  AFU050Device.cpp
  AFU050PropertyHandler.cpp
  AFU420Device.cpp
  AFU420DeviceProperties.cpp
  AFU420PropertyHandler.cpp
  libusb_utils.cpp
  UsbSession.cpp
  UsbHandler.cpp
  LibusbDevice.cpp
  libusblibrary.cpp
  )

include_directories("..")

find_package(libusb-1.0 REQUIRED QUIET)
include_directories(${LIBUSB_1_INCLUDE_DIRS})

add_library(tcam-libusb SHARED ${lib_libusb} $<TARGET_OBJECTS:tcam-base>)
target_link_libraries(tcam-libusb ${LIBUSB_1_LIBRARIES})
target_link_libraries(tcam-libusb tcam)

set_property(TARGET tcam-libusb PROPERTY VERSION ${TCAM_VERSION})
set_property(TARGET tcam-libusb PROPERTY SOVERSION ${TCAM_VERSION_MAJOR})

install(TARGETS tcam-libusb
  LIBRARY
  DESTINATION "${TCAM_INSTALL_LIB}/tcam-${TCAM_VERSION_MAJOR}"
  COMPONENT tcam-libusb)
