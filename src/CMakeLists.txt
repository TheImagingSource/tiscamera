
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

include( ${TCAM_SOURCE_DIR}/cmake/pkg-helper.cmake )

if (TCAM_BUILD_GST_1_0)
  add_subdirectory(gstreamer-1.0)
endif (TCAM_BUILD_GST_1_0)

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/tcam-config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/tcam-config.h")

if (TCAM_ENABLE_BASE_LIBRARIES)


add_library(tcam-base STATIC
  logging.cpp
  DeviceInfo.h
  DeviceInfo.cpp
  ImageBuffer.h
  ImageBuffer.cpp
  Allocator.h
  Allocator.cpp
  Memory.h
  Memory.cpp
  BufferPool.h
  BufferPool.cpp
  PropertyInterfaces.cpp

  SoftwareProperties.cpp
  SoftwarePropertiesExposureAuto.cpp
  SoftwarePropertiesBalanceWhite.cpp
  SoftwarePropertiesColorTransform.cpp
  SoftwarePropertiesImpl.cpp
  utils.cpp
  VideoFormat.cpp
  VideoFormatDescription.cpp
  ImageSink.cpp
  property_dependencies.h
  property_dependencies.cpp
  error.cpp
  devicelibrary.h
)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
  target_precompile_headers(tcam-base
    PRIVATE
	<vector>
	<string>
	<string_view>
	<algorithm>
	<outcome/result.hpp>
	)
endif()

target_link_libraries( tcam-base
PUBLIC
    dutils_img::img
    spdlog::spdlog
    outcome::outcome
    tcamprop1::base
PRIVATE
    dutils_img::pipe_auto
)

set_project_warnings(tcam-base)

set( srcs
  DeviceIndex.cpp
  Indexer.cpp
  DeviceInterface.cpp
  CaptureDevice.cpp
  CaptureDeviceImpl.cpp

  PropertyFilter.cpp

  public_utils.cpp

  libtcam_base.h
  libtcam_base.cpp
)


set(CMAKE_INSTALL_RPATH "${TCAM_INSTALL_LIB}/tcam-${TCAM_VERSION_MAJOR}")

set(TCAM_VERSION_FILE ${CMAKE_CURRENT_BINARY_DIR}/version.cpp)

# this creates version.cpp
add_custom_command(
  COMMAND ${CMAKE_COMMAND} -DVERSION_FILE_NAME=${TCAM_VERSION_FILE}
  -DARAVIS_VERSION="${ARAVIS_VERSION}"
  -DARAVIS_GIT_COMMIT_HASH="${ARAVIS_GIT_COMMIT_HASH}"
  -DTCAM_VERSION="${TCAM_VERSION}"
  -DTCAM_VERSION_MAJOR="${TCAM_VERSION_MAJOR}"
  -DTCAM_VERSION_MINOR="${TCAM_VERSION_MINOR}"
  -DTCAM_VERSION_PATCH="${TCAM_VERSION_PATCH}"
  -DTCAM_VERSION_MODIFIER="${TCAM_VERSION_MODIFIER}"
  -DGIT_BRANCH="${GIT_BRANCH}"
  -DGIT_COMMIT_HASH="${GIT_COMMIT_HASH}"
  -DGIT_COMMIT_COUNT="${GIT_COMMIT_COUNT}"
  -DENABLED_MODULES="${TCAM_ENABLED_MODULES}"
  -P "${TCAM_SOURCE_DIR}/cmake/create_version_file.cmake"
  COMMENT ""
  OUTPUT ${TCAM_VERSION_FILE}
  )


add_custom_target(generate-version-file
  DEPENDS ${TCAM_VERSION_FILE})


add_dependencies(tcam-base generate-version-file)

add_library(tcam SHARED ${srcs} ${TCAM_VERSION_FILE})

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
  target_precompile_headers(tcam REUSE_FROM tcam-base)
endif()

set_project_warnings(tcam)
set_property(TARGET tcam PROPERTY VERSION ${TCAM_VERSION})
set_property(TARGET tcam PROPERTY SOVERSION ${TCAM_VERSION_MAJOR})

target_include_directories( tcam
PRIVATE SYSTEM
    "${TCAM_SOURCE_DIR}/external/"      # TODO remove this
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}      # For version.h, version.cpp is generated an build in ${CMAKE_CURRENT_BINARY_DIR}, so we have to bend includes to that directory
)

# set_target_properties(tcam
#   PROPERTIES
#   CXX_VISIBILITY_PRESET hidden
#   )

target_link_libraries(tcam
  PUBLIC
  spdlog::spdlog
  tcam-base
  PRIVATE
  outcome::outcome
  tcamprop1::base
  )

if (TCAM_BUILD_VIRTCAM)
  add_subdirectory(virtcam)

  target_compile_definitions(tcam PRIVATE -DHAVE_VIRTCAM)

  target_link_libraries(tcam PRIVATE tcam-backend-virtcam)

endif (TCAM_BUILD_VIRTCAM)

if (TCAM_BUILD_V4L2)
  add_subdirectory(v4l2)

  target_compile_definitions(tcam PRIVATE -DHAVE_V4L2)

  target_link_libraries(tcam PRIVATE tcam-backend-v4l2)

endif (TCAM_BUILD_V4L2)


if (TCAM_BUILD_LIBUSB)
  add_subdirectory( libusb )

  target_compile_definitions( tcam PRIVATE -DHAVE_LIBUSB)

  target_link_libraries(tcam PRIVATE tcam-backend-libusb)

endif (TCAM_BUILD_LIBUSB)



if (TCAM_BUILD_ARAVIS)

  target_compile_definitions(tcam PRIVATE -DHAVE_ARAVIS)

  add_subdirectory(tcam-network)
  add_subdirectory(aravis)

  target_link_libraries(tcam PRIVATE tcam-backend-aravis)

endif (TCAM_BUILD_ARAVIS)


install(TARGETS tcam
  LIBRARY
  DESTINATION "${TCAM_INSTALL_LIB}"
  COMPONENT bin)

endif (TCAM_ENABLE_BASE_LIBRARIES)
