
# Copyright 2014 The Imaging Source Europe GmbH
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

find_package(libusb-1.0 REQUIRED QUIET)

add_library( tcam-backend-libusb STATIC
  AFU050Device.cpp
  AFU050DeviceBackend.cpp
  AFU050PropertyImpl.cpp
  AFU420Device.cpp
  AFU420PropertyImpl.cpp
  AFU420DeviceBackend.cpp
  AFU420DeviceProperties.cpp
  libusb_utils.cpp
  UsbSession.cpp
  UsbHandler.cpp
  LibusbDevice.cpp
  libusb_api.cpp
  )


target_link_libraries(tcam-backend-libusb
  PUBLIC
  tcam-base
  )

target_include_directories(tcam-backend-libusb PRIVATE ${LIBUSB_1_INCLUDE_DIRS})
target_link_libraries(tcam-backend-libusb PRIVATE ${LIBUSB_1_LIBRARIES})

set_project_warnings(tcam-backend-libusb)
