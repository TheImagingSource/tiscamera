
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


add_subdirectory(gobject)


if (BUILD_GST_1_0)
  add_subdirectory(gstreamer-1.0)
endif (BUILD_GST_1_0)

add_subdirectory(algorithms)

# If you really want to set additional options from outside CMakeLists.txt you
# can do this:
# add_defintions($ENV{CFLAGS})

find_package(TinyXML REQUIRED QUIET)
include_directories(${TinyXML_INCLUDE_DIR})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(cflags "-fpic -w -Wall -Wextra -pedantic -Wundef -Wshadow")
set(lflags "-Wl,--gc-sections,--no-as-needed -ffunction-sections -fdata-sections -ldl")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cflags} ${lflags}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${cflags} ${lflags} -pthread")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/version.h)

set(base
  format.cpp
  logging.cpp
  DeviceInfo.cpp
  image_transform_base.h
  MemoryBuffer.cpp
  Properties.cpp
  Property.cpp
  PropertyGeneration.cpp
  utils.cpp
  VideoFormat.cpp
  VideoFormatDescription.cpp
  ImageSink.cpp)

add_library(tcam-base OBJECT ${base})

target_compile_definitions(tcam-base PRIVATE TCAM_LOG_ENV_NAME="TCAM_LOG")

set( srcs
  BackendLoader.cpp
  DeviceIndex.cpp
  DeviceInterface.cpp
  CaptureDevice.cpp
  CaptureDeviceImpl.cpp
  PipelineManager.cpp
  ImageSource.cpp
  serialization.cpp
  PropertyHandler.cpp
  public_utils.cpp
)

if (BUILD_V4L2)
  add_subdirectory(v4l2)
endif (BUILD_V4L2)

if (BUILD_ARAVIS)
  add_subdirectory(tcam-network)
  add_subdirectory(aravis)
endif (BUILD_ARAVIS)

if (BUILD_LIBUSB)
  add_subdirectory(libusb)
endif (BUILD_LIBUSB)


set(CMAKE_INSTALL_RPATH "${TCAM_INSTALL_LIB}/tcam-${TCAM_VERSION_MAJOR}")

set(PUBLIC_HEADER tcam.h)
add_library(tcam SHARED ${srcs} $<TARGET_OBJECTS:tcam-base>)

set_property(TARGET tcam PROPERTY VERSION ${TCAM_VERSION})
set_property(TARGET tcam PROPERTY SOVERSION ${TCAM_VERSION_MAJOR})


TARGET_LINK_LIBRARIES(tcam ${TinyXML_LIBRARIES})

# install(FILES ${PUBLIC_HEADER}
#   DESTINATION "${TCAM_INSTALL_INCLUDE}")

install(TARGETS tcam
  LIBRARY
  DESTINATION "${TCAM_INSTALL_LIB}"
  COMPONENT tcam)
