
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


#Specify the version being used aswell as the language
cmake_minimum_required(VERSION 2.6)

#Name your project here
project(tcam)

set(TCAM_VERSION_MAJOR 0)
set(TCAM_VERSION_MINOR 9)
set(TCAM_VERSION_PATCH 1)

set(TCAM_VERSION "${TCAM_VERSION_MAJOR}.${TCAM_VERSION_MINOR}.${TCAM_VERSION_PATCH}" CACHE STRING "Version number")

set(TCAM_ORIGIN "https://github.com/TheImagingSource/tiscamera")

# Generate a git revision description
#
# Get the current working branch
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
#
# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
#
# Get commit count
execute_process(
  COMMAND git rev-list --count HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_COUNT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)


# assure config.h is found
include_directories("${PROJECT_BINARY_DIR}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)


option(BUILD_GST_1_0  "Build gstreamer-1.0 plugins?"         ON)
option(BUILD_ARAVIS   "Include GigE support via aravis"      OFF)
option(BUILD_V4L2     "Include support for USB cameras"      ON )
option(BUILD_LIBUSB   "Include support for LibUsb cameras"   ON )
option(BUILD_TOOLS    "Build additional utilities"           OFF)

set(TCAM_INTERNAL_ARAVIS ON CACHE STRING "Use internal aravis dependency instead of system libraries")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

if (BUILD_V4L2)
  add_definitions(-DHAVE_USB)
  set(HAVE_USB 1)
endif (BUILD_V4L2)

set(TCAM_INSTALL_LIB "${CMAKE_INSTALL_PREFIX}/lib" CACHE STRING "library installation path" FORCE)
set(TCAM_INSTALL_INCLUDE "${CMAKE_INSTALL_PREFIX}/include" CACHE STRING "header installation path" FORCE)
set(TCAM_INSTALL_BIN "${CMAKE_INSTALL_PREFIX}/bin" CACHE STRING "binary installation path" FORCE)
set(TCAM_INSTALL_UDEV "/usr/lib/udev/rules.d" CACHE STRING "udev rules installation path")
set(TCAM_INSTALL_UVCDYNCTRL "/usr/share/uvcdynctrl/data/199e" CACHE STRING "uvcdynctrl rules installation path")
set(TCAM_INSTALL_SYSTEMD "/lib/systemd/system/" CACHE STRING "systemd unit installation path" FORCE)
set(TCAM_INSTALL_PKGCONFIG "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" CACHE STRING "pkgconfig installation path" FORCE)

add_subdirectory(dependencies)
add_subdirectory(src)


add_subdirectory(data)


if (BUILD_TOOLS)
  add_subdirectory(tools)
endif (BUILD_TOOLS)


# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)


add_subdirectory(packaging)

# ensure we have the correct install directories
# subdirectories make additional checks to find better suited install directories
# to prevent the user having to set environment variables, etc
get_directory_property(TCAM_INSTALL_GIR  DIRECTORY ${CMAKE_SOURCE_DIR}/src/gobject  DEFINITION TCAM_INSTALL_GIR)
get_directory_property(TCAM_INSTALL_TYPELIBDIR  DIRECTORY ${CMAKE_SOURCE_DIR}/src/gobject  DEFINITION TCAM_INSTALL_TYPELIBDIR)
get_directory_property(TCAM_INSTALL_GST_1_0  DIRECTORY ${CMAKE_SOURCE_DIR}/src/gstreamer-1.0  DEFINITION TCAM_INSTALL_GST_1_0)

#
#
#

# Give overview over build configuration

MESSAGE(STATUS "")
MESSAGE(STATUS "Build gstreamer-1.0 plugins:   " ${BUILD_GST_1_0})
MESSAGE(STATUS "Support for GigE via aravis:   " ${BUILD_ARAVIS})
MESSAGE(STATUS "Support for USB cameras:       " ${BUILD_V4L2})
MESSAGE(STATUS "Support for LibUsb cameras:    " ${BUILD_LIBUSB})
MESSAGE(STATUS "Build additional utilities:    " ${BUILD_TOOLS})
MESSAGE(STATUS "")
MESSAGE(STATUS "Installation prefix:           " ${CMAKE_INSTALL_PREFIX})
MESSAGE(STATUS "Installing binaries to:        " ${TCAM_INSTALL_BIN})
MESSAGE(STATUS "Installing libraries to        " ${TCAM_INSTALL_LIB})
MESSAGE(STATUS "Installing header to:          " ${TCAM_INSTALL_INCLUDE})
MESSAGE(STATUS "Installing udev rules to:      " ${TCAM_INSTALL_UDEV})
MESSAGE(STATUS "Installing uvcdynctrl rules to:" ${TCAM_INSTALL_UVCDYNCTRL})
MESSAGE(STATUS "Installing gstreamer-1.0 to:   " ${TCAM_INSTALL_GST_1_0})
MESSAGE(STATUS "Installing gobject-introspection to: " ${TCAM_INSTALL_GIR})
MESSAGE(STATUS "Installing introspection typelib to: " ${TCAM_INSTALL_TYPELIB})
MESSAGE(STATUS "")
