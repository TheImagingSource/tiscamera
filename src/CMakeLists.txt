
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


add_subdirectory(lib/dutils_image)


# helper funcion to receive the value of an arbitrary variable
function(pkg_check_variable _pkg _name)
  string(TOUPPER ${_pkg} _pkg_upper)
  string(TOUPPER ${_name} _name_upper)
  string(REPLACE "-" "_" _pkg_upper ${_pkg_upper})
  string(REPLACE "-" "_" _name_upper ${_name_upper})
  set(_output_name "${_pkg_upper}_${_name_upper}")

  execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=${_name} ${_pkg}
    OUTPUT_VARIABLE _pkg_result
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  set("${_output_name}" "${_pkg_result}" CACHE STRING "pkg-config variable ${_name} of ${_pkg}")
endfunction()


add_subdirectory(gobject)

if (BUILD_GST_1_0)
  add_subdirectory(gstreamer-1.0)
endif (BUILD_GST_1_0)

if (TCAM_ENABLE_BASE_LIBRARIES)

add_subdirectory(algorithms)


set(base
  format.cpp
  logging.cpp
  DeviceInfo.cpp
  ImageBuffer.cpp
  PropertyInterfaces.cpp
  PropertyCategory.cpp
  SoftwarePropertiesBase.cpp
  SoftwareProperties.cpp
  SoftwarePropertyBackend.cpp
  SoftwarePropertiesImpl.cpp
  utils.cpp
  VideoFormat.cpp
  VideoFormatDescription.cpp
  ImageSink.cpp
  json.cpp
  error.cpp
  )

add_library(tcam-base OBJECT ${base})

target_compile_definitions(tcam-base PRIVATE TCAM_LOG_ENV_NAME="TCAM_LOG")
target_include_directories(tcam-base PRIVATE "${TCAM_SOURCE_DIR}/external/spdlog/include")
target_include_directories(tcam-base PRIVATE "${TCAM_SOURCE_DIR}/src/include/")
set_project_warnings(tcam-base)
set( srcs
  BackendLoader.cpp
  DeviceIndex.cpp
  Indexer.cpp
  DeviceInterface.cpp
  CaptureDevice.cpp
  CaptureDeviceImpl.cpp
  PipelineManager.cpp
  PropertyFilter.cpp
  ImageSource.cpp
  public_utils.cpp
)

if (BUILD_V4L2)
  add_subdirectory(v4l2)

  set(srcs
    ${srcs}
    v4l2/V4l2Device.cpp
    v4l2/V4L2PropertyBackend.cpp
    v4l2/V4L2DeviceProperties.cpp
    v4l2/v4l2_property_impl.cpp
    v4l2/v4l2_genicam_mapping.cpp
    v4l2/v4l2_genicam_conversion.cpp
    v4l2/v4l2_utils.cpp
    v4l2/v4l2_api.cpp
    )

endif (BUILD_V4L2)

if (BUILD_ARAVIS)
  add_subdirectory(tcam-network)

  set(srcs
    ${srcs}
    aravis/AravisDevice.cpp
    aravis/AravisPropertyBackend.cpp
    aravis/AravisDeviceProperties.cpp
    aravis/aravis_property_impl.cpp
    aravis/aravis_utils.cpp
    aravis/aravis_api.cpp
    )


  if (NOT TCAM_INTERNAL_ARAVIS)

    find_package(Aravis REQUIRED QUIET)

  endif(NOT TCAM_INTERNAL_ARAVIS)


endif (BUILD_ARAVIS)

if (BUILD_LIBUSB)
  set(srcs
    ${srcs}
    libusb/AFU050Device.cpp
    libusb/AFU050DeviceBackend.cpp
    libusb/AFU050PropertyImpl.cpp
    libusb/AFU420Device.cpp
    libusb/AFU420PropertyImpl.cpp
    libusb/AFU420DeviceBackend.cpp
    libusb/AFU420DeviceProperties.cpp
    libusb/libusb_utils.cpp
    libusb/UsbSession.cpp
    libusb/UsbHandler.cpp
    libusb/LibusbDevice.cpp
    libusb/libusb_api.cpp
    )

  add_definitions(-DHAVE_LIBUSB)
  find_package(libusb-1.0 REQUIRED QUIET)

endif (BUILD_LIBUSB)

include_directories(SYSTEM "${TCAM_SOURCE_DIR}/external/")


set(CMAKE_INSTALL_RPATH "${TCAM_INSTALL_LIB}/tcam-${TCAM_VERSION_MAJOR}")

set(PUBLIC_HEADER tcam.h)


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


add_library(tcam SHARED ${srcs} $<TARGET_OBJECTS:tcam-base> ${TCAM_VERSION_FILE})
target_include_directories(tcam PRIVATE "${TCAM_SOURCE_DIR}/src/include/")
target_include_directories(tcam PRIVATE "${TCAM_SOURCE_DIR}/external/spdlog/include")

add_dependencies(tcam-base generate-version-file)
set_project_warnings(tcam)
set_property(TARGET tcam PROPERTY VERSION ${TCAM_VERSION})
set_property(TARGET tcam PROPERTY SOVERSION ${TCAM_VERSION_MAJOR})

# set_target_properties(tcam
#   PROPERTIES
#   POSITION_INDEPENDENT_CODE ON
#   CXX_VISIBILITY_PRESET hidden
#   )

#target_link_options(tcam PUBLIC "-Wl,--exclude-libs,ALL")

# target_link_libraries(tcam dl)
target_link_libraries(tcam PRIVATE spdlog)
target_link_libraries(tcam PRIVATE tcam-algorithms)


# add_dependencies(tcam-base version-header)

if (BUILD_V4L2)


  find_package(UDev REQUIRED QUIET)
  include_directories(tcam PRIVATE ${UDEV_INCLUDE_DIR})

  target_compile_definitions(tcam PRIVATE -DHAVE_UDEV)
  target_compile_definitions(tcam PRIVATE -DHAVE_V4L2)

  find_package(libusb-1.0 REQUIRED QUIET)
  target_include_directories(tcam PRIVATE ${LIBUSB_1_INCLUDE_DIRS})

  target_link_libraries(tcam PRIVATE ${UDEV_LIBRARIES} ${LIBUSB_1_LIBRARIES})
  target_link_libraries(tcam PRIVATE tcam-dfk73)

endif (BUILD_V4L2)


if (BUILD_ARAVIS)

  target_compile_definitions(tcam PRIVATE -DHAVE_ARAVIS)

  find_package(GObject REQUIRED QUIET) # dependency
  find_package(GLIB2   REQUIRED QUIET) # dependency
  find_package(GIO     REQUIRED QUIET) # dependency

  target_include_directories(tcam PRIVATE ${GOBJECT_INCLUDE_DIR})
  target_include_directories(tcam PRIVATE ${GLIB2_INCLUDE_DIR})
  target_include_directories(tcam PRIVATE "${TCAM_SOURCE_DIR}/tools/gige-daemon")

  if (TCAM_INTERNAL_ARAVIS)
    add_dependencies(tcam aravis)
    set(ARAVIS_LIB "${TCAM_BINARY_DIR}/external/aravis-prefix/src/aravis-build/src/.libs/libaravis-0.6.a")

    find_package(LibXml2 REQUIRED QUIET)
    find_package(PCAP REQUIRED QUIET)

    target_include_directories(tcam PRIVATE "${GIO_INCLUDE_DIR}")
    target_include_directories(tcam SYSTEM PRIVATE "${TCAM_SOURCE_DIR}/external/aravis/src")
    target_include_directories(tcam PRIVATE SYSTEM "${TCAM_BINARY_DIR}/external/aravis-prefix/src/aravis-build/src/")

    target_link_libraries(tcam PRIVATE "${ARAVIS_LIB}")
    target_link_libraries(tcam PRIVATE "${LIBXML2_LIBRARIES}")
    target_link_libraries(tcam PRIVATE "${GIO_LIBRARIES}")
    target_link_libraries(tcam PRIVATE "${PCAP_LIBRARIES}")

  else (TCAM_INTERNAL_ARAVIS)

    target_include_directories(tcam PRIVATE SYSTEM ${aravis_INCLUDE_DIR})
    target_link_libraries(tcam PRIVATE ${aravis_LIBRARIES})

  endif (TCAM_INTERNAL_ARAVIS)

  if (TCAM_ARAVIS_USB_VISION)

    find_package(libusb-1.0 REQUIRED QUIET)
    target_include_directories(tcam PRIVATE "${LIBUSB_1_INCLUDE_DIRS}")
    target_link_libraries(tcam PRIVATE "${LIBUSB_1_LIBRARIES}")

  endif (TCAM_ARAVIS_USB_VISION)

endif (BUILD_ARAVIS)


if (BUILD_LIBUSB)
  target_include_directories(tcam PRIVATE ${LIBUSB_1_INCLUDE_DIRS})
  target_link_libraries(tcam PRIVATE ${LIBUSB_1_LIBRARIES})

endif (BUILD_LIBUSB)

install(TARGETS tcam
  LIBRARY
  DESTINATION "${TCAM_INSTALL_LIB}"
  COMPONENT tcam)

endif (TCAM_ENABLE_BASE_LIBRARIES)
