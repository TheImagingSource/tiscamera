
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
set(TCAM_VERSION_MINOR 2)
set(TCAM_VERSION_PATCH 0)

set(TCAM_VERSION "${TCAM_VERSION_MAJOR}.${TCAM_VERSION_MINOR}.${TCAM_VERSION_PATCH}" CACHE STRING "Version number")

set(TCAM_ORIGIN "https://github.com/TheImagingSource/tiscamera")

# assure config.h is found
include_directories("${PROJECT_BINARY_DIR}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)


option(BUILD_GST_1_0  "Build gstreamer-1.0 plugins?"         ON)
option(BUILD_ARAVIS   "Include GigE support via aravis"      OFF)
option(BUILD_V4L2     "Include support for USB cameras"      ON )
option(BUILD_TOOLS    "Build additional utilities"           OFF)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

if (BUILD_V4L2)
  add_definitions(-DHAVE_USB)
  set(HAVE_USB 1)
endif (BUILD_V4L2)

set(TCAM_INSTALL_LIB "${CMAKE_INSTALL_PREFIX}/lib" CACHE STRING "library installation path")
set(TCAM_INSTALL_INCLUDE "${CMAKE_INSTALL_PREFIX}/include" CACHE STRING "header installation path")
set(TCAM_INSTALL_BIN "${CMAKE_INSTALL_PREFIX}/bin" CACHE STRING "binary installation path")
set(TCAM_INSTALL_UDEV "/usr/lib/udev/rules.d" CACHE STRING "udev rules installation path")
set(TCAM_INSTALL_UVCDYNCTRL "/usr/share/uvcdynctrl/data/199e" CACHE STRING "uvcdynctrl rules installation path")
set(TCAM_INSTALL_GST_1_0 "${CMAKE_INSTALL_PREFIX}/lib/gstreamer-1.0" CACHE STRING "gstreamer-1.0 module installation path")

set(TCAM_INSTALL_GIR "${CMAKE_INSTALL_PREFIX}/share/gir-1.0/" CACHE STRING "gobject introspection installation path")
set(TCAM_INSTALL_TYPELIB "${CMAKE_INSTALL_PREFIX}/lib/girepository-1.0" CACHE STRING "gobject introspection typelib installation path")

add_subdirectory(src)


add_subdirectory(data)


if (BUILD_TOOLS)
  add_subdirectory(tools)
endif (BUILD_TOOLS)


# Give overview over build configuration

MESSAGE(STATUS "")
MESSAGE(STATUS "Build gstreamer-1.0 plugins:   " ${BUILD_GST_1_0})
MESSAGE(STATUS "Support for GigE via aravis:   " ${BUILD_ARAVIS})
MESSAGE(STATUS "Support for USB cameras:       " ${BUILD_V4L2})
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
