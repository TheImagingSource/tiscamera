

#Specify the version being used aswell as the language
cmake_minimum_required(VERSION 2.6)

#Name your project here
project(tiscamera)

set(TISCAMERA_VERSION 1.0.0)

# configure_file(
#   "${PROJECT_SOURCE_DIR}/src/config.h.in"
#   "${PROJECT_BINARY_DIR}/config.h")

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
option(BUILD_FIREWIRE "Include support for firewire devices" OFF)
option(BUILD_TOOLS    "Build additional utilities"           OFF)
option(BUILD_GUI      "Build gui versions of tools"          ON)
option(BUILD_QT4      "Build Qt 4 libraries"                 ON)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -flto -pthread")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto -pthread")

if (BUILD_USB)
  add_definitions(-DHAVE_USB)
endif (BUILD_USB)

add_subdirectory(src)

if (BUILD_GUI)
  add_subdirectory(vision)
endif (BUILD_GUI)

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
