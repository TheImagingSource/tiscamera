
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


# If you really want to set additional options from outside CMakeLists.txt you
# can do this:
# add_defintions($ENV{CFLAGS})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(gobject)


if (BUILD_GST_1_0)
  add_subdirectory(gstreamer-1.0)
endif (BUILD_GST_1_0)

add_subdirectory(algorithms)


set(base
  format.cpp
  logging.cpp
  DeviceInfo.cpp
  image_transform_base.h
  ImageBuffer.cpp
  Properties.cpp
  Property.cpp
  PropertyGeneration.cpp
  utils.cpp
  VideoFormat.cpp
  VideoFormatDescription.cpp
  ImageSink.cpp
  json.cpp
  )

add_library(tcam-base OBJECT ${base})

target_compile_definitions(tcam-base PRIVATE TCAM_LOG_ENV_NAME="TCAM_LOG")

set( srcs
  BackendLoader.cpp
  LibraryHandle.cpp
  DeviceIndex.cpp
  Indexer.cpp
  DeviceInterface.cpp
  CaptureDevice.cpp
  CaptureDeviceImpl.cpp
  PipelineManager.cpp
  ImageSource.cpp
  PropertyHandler.cpp
  public_utils.cpp
)

if (BUILD_V4L2)
  add_subdirectory(v4l2)

  set(srcs
    ${srcs}
    v4l2/V4l2Device.cpp
    v4l2/V4L2PropertyHandler.cpp
    v4l2/v4l2_utils.cpp
    # v4l2/v4l2library.cpp
    v4l2/v4l2_api.cpp)

  find_package(UDev REQUIRED QUIET)
  include_directories(${UDEV_INCLUDE_DIR})
  add_definitions(-DHAVE_UDEV)
  add_definitions(-DHAVE_V4L2)

  find_package(libusb-1.0 REQUIRED QUIET)
  include_directories(${LIBUSB_1_INCLUDE_DIRS})


endif (BUILD_V4L2)

if (BUILD_ARAVIS)
  add_subdirectory(tcam-network)
  # add_subdirectory(aravis)

  add_definitions(-DHAVE_ARAVIS)
  set(srcs
    ${srcs}
    aravis/AravisDevice.cpp
    aravis/aravis_utils.cpp
    aravis/aravis_api.cpp
)

find_package(GObject REQUIRED QUIET) # dependency
find_package(GLIB2   REQUIRED QUIET) # dependency
find_package(GIO     REQUIRED QUIET) # dependency

if (TCAM_INTERNAL_ARAVIS)


  include_directories("${GIO_INCLUDE_DIR}")
  include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/dependencies/aravis/src")
  include_directories(SYSTEM "${CMAKE_BINARY_DIR}/dependencies/aravis-prefix/src/aravis-build/src/")

else (TCAM_INTERNAL_ARAVIS)

  find_package(Aravis REQUIRED QUIET)
  include_directories(${aravis_INCLUDE_DIR})

endif(TCAM_INTERNAL_ARAVIS)

include_directories(${GOBJECT_INCLUDE_DIR})
include_directories(${GLIB2_INCLUDE_DIR})
include_directories("${CMAKE_SOURCE_DIR}/tools/gige-daemon")




endif (BUILD_ARAVIS)

if (BUILD_LIBUSB)
  # add_subdirectory(libusb)
  set(srcs
    ${srcs}
    libusb/AFU050Device.cpp
    libusb/AFU050PropertyHandler.cpp
    libusb/AFU420Device.cpp
    libusb/AFU420DeviceProperties.cpp
    libusb/AFU420PropertyHandler.cpp
    libusb/libusb_utils.cpp
    libusb/UsbSession.cpp
    libusb/UsbHandler.cpp
    libusb/LibusbDevice.cpp
    libusb/libusb_api.cpp
    # libusb/libusblibrary.cpp
    )

  add_definitions(-DHAVE_LIBUSB)
  find_package(libusb-1.0 REQUIRED QUIET)
  include_directories(${LIBUSB_1_INCLUDE_DIRS})

endif (BUILD_LIBUSB)

include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/dependencies/")


set(CMAKE_INSTALL_RPATH "${TCAM_INSTALL_LIB}/tcam-${TCAM_VERSION_MAJOR}")

set(PUBLIC_HEADER tcam.h)
add_library(tcam SHARED ${srcs} $<TARGET_OBJECTS:tcam-base>)

set_property(TARGET tcam PROPERTY VERSION ${TCAM_VERSION})
set_property(TARGET tcam PROPERTY SOVERSION ${TCAM_VERSION_MAJOR})

TARGET_LINK_LIBRARIES(tcam dl)


set(TCAM_VERSION_HEADER ${PROJECT_BINARY_DIR}/version.h)

add_custom_target(
  version-header
  COMMAND ${CMAKE_COMMAND} -DVERSION_FILE_NAME=${TCAM_VERSION_HEADER}
  -DARAVIS_VERSION="${ARAVIS_VERSION}"
  -DARAVIS_GIT_COMMIT_HASH="${ARAVIS_GIT_COMMIT_HASH}"
  -DTCAM_VERSION="${TCAM_VERSION}"
  -DGIT_BRANCH="${GIT_BRANCH}"
  -DGIT_COMMIT_HASH="${GIT_COMMIT_HASH}"
  -DGIT_COMMIT_COUNT="${GIT_COMMIT_COUNT}"
  -P "${CMAKE_SOURCE_DIR}/cmake/create_version_header.cmake"
  COMMENT ""
  OUTPUT ${TCAM_VERSION_HEADER}
  )

add_dependencies(tcam-base   version-header)

if (BUILD_V4L2)

    target_link_libraries(tcam ${UDEV_LIBRARIES} ${LIBUSB_1_LIBRARIES})
    target_link_libraries(tcam tcam-dfk73)

endif (BUILD_V4L2)


if (BUILD_ARAVIS)

  if (TCAM_INTERNAL_ARAVIS)
    add_dependencies(tcam aravis)
    set(ARAVIS_LIB "${CMAKE_BINARY_DIR}/dependencies/aravis-prefix/src/aravis-build/src/.libs/libaravis-0.6.a")

    find_package(LibXml2 REQUIRED QUIET)
    find_package(PCAP REQUIRED QUIET)

    target_link_libraries(tcam "${ARAVIS_LIB}")

    target_link_libraries(tcam "${LIBXML2_LIBRARIES}")
    target_link_libraries(tcam "${GIO_LIBRARIES}")
    target_link_libraries(tcam "${PCAP_LIBRARIES}")

  else (TCAM_INTERNAL_ARAVIS)

    target_link_libraries(tcam ${aravis_LIBRARIES})

  endif (TCAM_INTERNAL_ARAVIS)

  if (TCAM_ARAVIS_USB_VISION)

    find_package(libusb-1.0 REQUIRED QUIET)
    target_link_libraries(tcam "${LIBUSB_1_LIBRARIES}")

  endif (TCAM_ARAVIS_USB_VISION)

endif (BUILD_ARAVIS)


if (BUILD_LIBUSB)

  target_link_libraries(tcam ${LIBUSB_1_LIBRARIES})

endif (BUILD_LIBUSB)

install(TARGETS tcam
  LIBRARY
  DESTINATION "${TCAM_INSTALL_LIB}"
  COMPONENT tcam)
