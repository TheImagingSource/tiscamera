# Copyright 2021 The Imaging Source Europe GmbH
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

find_package(LibUuid REQUIRED QUIET)

add_library(tcam-uvc-extension STATIC
  "uvc-extension-loader.h"
  "uvc-extension-loader.cpp"
  )
set_project_warnings(tcam-uvc-extension)

target_link_libraries(tcam-uvc-extension ${LIBUUID_LIBRARIES})

target_include_directories(tcam-uvc-extension
  PRIVATE ${LIBUUID_INCLUDE_DIRS}
  PRIVATE "${TCAM_BINARY_DIR}/"
  PRIVATE "${TCAM_SOURCE_DIR}/external/json")


if (NOT TCAM_BUILD_UVC_EXTENSION_LOADER_ONLY)

find_package(UDev REQUIRED QUIET)

add_library(tcam-backend-v4l2 STATIC
  V4l2Device.cpp
  V4l2Device.h
  V4L2Allocator.h
  V4L2Allocator.cpp
  V4L2PropertyBackend.cpp
  V4L2PropertyBackend.h
  V4L2DeviceProperties.cpp
  v4l2_property_impl.cpp
  v4l2_property_impl.h
  v4l2_genicam_mapping.cpp
  v4l2_genicam_mapping.h
  v4l2_genicam_conversion.cpp
  v4l2_genicam_conversion.h
  v4l2_utils.cpp
  v4l2_utils.h
  v4l2_api.cpp
  v4l2_api.h

  sensor_id_33u.h
  )

target_link_libraries(tcam-backend-v4l2
  PUBLIC
  tcam-base
  PRIVATE
  tcam-uvc-extension
  tcamprop1::base
  ${UDEV_LIBRARIES} ${LIBUSB_1_LIBRARIES}
  )

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
  target_precompile_headers(tcam-backend-v4l2 REUSE_FROM tcam-base)
endif()

target_compile_definitions(tcam-backend-v4l2 PRIVATE -DHAVE_UDEV)

target_include_directories(tcam-backend-v4l2
  PRIVATE
  ${UDEV_INCLUDE_DIR}
  )

set_project_warnings(tcam-backend-v4l2)

endif (NOT TCAM_BUILD_UVC_EXTENSION_LOADER_ONLY)
