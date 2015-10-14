
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

set(TCAM_VERSION ${TCAM_VERSION_MAJOR}.${TCAM_VERSION_MINOR}.${TCAM_VERSION_PATCH} CACHE STRING "Version number")

# assure config.h is found
include_directories("${PROJECT_BINARY_DIR}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)


option(BUILD_DOC      "Generate documentation?"              OFF)
option(BUILD_GST_0_10 "Build gstreamer-0.10 plugins?"        OFF)
option(BUILD_GST_1_0  "Build gstreamer-1.0 plugins?"         OFF)
option(BUILD_ARAVIS   "Include GigE support via aravis"      OFF)
option(BUILD_USB      "Include support for USB cameras"      ON )
option(BUILD_TOOLS    "Build additional utilities"           OFF)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

if (BUILD_USB)
  add_definitions(-DHAVE_USB)
  set(HAVE_USB 1)
endif (BUILD_USB)

add_subdirectory(src)


add_subdirectory(data)


if (BUILD_TOOLS)
  add_subdirectory(tools)
endif (BUILD_TOOLS)

if (BUILD_DOC)
  add_subdirectory(doc)
endif (BUILD_DOC)

# Give overview over build configuration

MESSAGE(STATUS "")
MESSAGE(STATUS "Generate documentation:        " ${BUILD_DOC})
MESSAGE(STATUS "Build gstreamer-0.10 plugins:  " ${BUILD_GST_0_10})
MESSAGE(STATUS "Build gstreamer-1.0 plugins:   " ${BUILD_GST_1_0})
MESSAGE(STATUS "Support for GigE via aravis:   " ${BUILD_ARAVIS})
MESSAGE(STATUS "Support for USB cameras:       " ${BUILD_USB})
MESSAGE(STATUS "Support for firewire devices:  " ${BUILD_FIREWIRE})
MESSAGE(STATUS "Build additional utilities:    " ${BUILD_TOOLS})
MESSAGE(STATUS "")
