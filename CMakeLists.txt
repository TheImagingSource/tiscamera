
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
cmake_minimum_required(VERSION 3.2)

#Name your project here
project(tcam)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(version.cmake)

set(TCAM_VERSION "${TCAM_VERSION_MAJOR}.${TCAM_VERSION_MINOR}.${TCAM_VERSION_PATCH}${TCAM_VERSION_MODIFIER}" CACHE STRING "Version number")

set(TCAM_ORIGIN "https://github.com/TheImagingSource/tiscamera")

include("CompilerWarnings")
include("git-helper")

git_commit_hash(GIT_COMMIT_HASH)
git_commit_count(GIT_COMMIT_COUNT)
git_branch(GIT_BRANCH)
git_commit_tag(GIT_TAG)

set(default_build_type "Release")

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
    STRING "Choose the type of build." FORCE)

  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(TCAM_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(TCAM_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TCAM_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${TCAM_BINARY_DIR}/lib)


# assure config.h is found
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX "/usr/" CACHE PATH "Common prefix for all installed files." FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# All option for configuration come from here
include(CMakeOptions.cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -pedantic -pthread -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -pthread -fPIC")

add_compile_options( -Wno-psabi )   # disable ABI change message of g++8


include(CMakeInstall.cmake)

include("clang-format")


if (TCAM_BUILD_UVC_EXTENSION_LOADER_ONLY)

  add_subdirectory(src/v4l2)
  add_subdirectory(tools/tcam-uvc-extension-loader/)
  add_subdirectory(data/uvc-extensions/)
  add_subdirectory(data/udev/)

endif (TCAM_BUILD_UVC_EXTENSION_LOADER_ONLY)

if (TCAM_BUILD_FIRMWARE_UPDATE_ONLY)

  add_subdirectory(external)
  add_subdirectory(tools/firmware-update)

endif (TCAM_BUILD_FIRMWARE_UPDATE_ONLY)


if (TCAM_BUILD_CAMERA_IP_CONF_ONLY)

  add_subdirectory(external)
  add_subdirectory(src/tcam-network/)
  add_subdirectory(tools/camera-ip-conf)

endif (TCAM_BUILD_CAMERA_IP_CONF_ONLY)


if (NOT TCAM_EXCLUSIVE_BUILD)

  # TODO: these should not always be included
  add_subdirectory(external)
  add_subdirectory(src)

  if (TCAM_ENABLE_DATA_INSTALL)
    add_subdirectory(data)
    # TODO: this should not be always on
    install(DIRECTORY examples
      DESTINATION "${CMAKE_INSTALL_PREFIX}/share/theimagingsource/tiscamera/")
  endif (TCAM_ENABLE_DATA_INSTALL)


  if (BUILD_TOOLS)
    add_subdirectory(tools)
  endif (BUILD_TOOLS)

  if (BUILD_DOCUMENTATION)
    add_subdirectory(doc)
  endif (BUILD_DOCUMENTATION)

  if (BUILD_TESTS)
    enable_testing()

    add_subdirectory(tests)
  endif (BUILD_TESTS)

  if (BUILD_GST_1_0)

    # ensure we have the correct install directories
    # subdirectories make additional checks to find better suited install directories
    # to prevent the user having to set environment variables, etc
    get_directory_property(TCAM_INSTALL_GIR  DIRECTORY ${TCAM_SOURCE_DIR}/src/gobject  DEFINITION TCAM_INSTALL_GIR)
    get_directory_property(TCAM_INSTALL_TYPELIBDIR  DIRECTORY ${TCAM_SOURCE_DIR}/src/gobject  DEFINITION TCAM_INSTALL_TYPELIBDIR)
    get_directory_property(TCAM_INSTALL_GST_1_0  DIRECTORY ${TCAM_SOURCE_DIR}/src/gstreamer-1.0  DEFINITION TCAM_INSTALL_GST_1_0)

  endif (BUILD_GST_1_0)

endif (NOT TCAM_EXCLUSIVE_BUILD)

# uninstall target
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

# create target uninstall to deinstall tiscamera
# if tiscamera is a subproject/module
# create uninstall-tiscamera instead
if (TARGET uninstall)
  add_custom_target(uninstall-tiscamera
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
else (TARGET uninstall)
  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif (TARGET uninstall)

# is never installed, etc. can come last

add_subdirectory(packaging)
add_subdirectory(scripts)



#
#
#

# Give overview over build configuration

print_install_overview()
