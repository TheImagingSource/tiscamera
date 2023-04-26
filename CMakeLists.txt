
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
cmake_minimum_required(VERSION 3.10)

#Name your project here
project(tcam)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(version.cmake)

set(TCAM_VERSION "${TCAM_VERSION_MAJOR}.${TCAM_VERSION_MINOR}.${TCAM_VERSION_PATCH}${TCAM_VERSION_MODIFIER}" CACHE STRING "Version number")


set(TCAM_ORIGIN "https://github.com/TheImagingSource/tiscamera")

include("cmake/CompilerWarnings.cmake")
include("cmake/git-helper.cmake")

set(default_build_type "Release")   # Override the default build type to be 'Release' and not 'RelWithDebInfo'
include("cmake/StandardProjectSettings.cmake")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

find_git_settings() # exports GIT_COMMIT_COUNT, GIT_TAG, ...

set(TCAM_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(TCAM_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TCAM_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${TCAM_BINARY_DIR}/lib)

if( ${GIT_REPO_PRESENT} )
    message(STATUS "TCAM_VERSION: ${TCAM_VERSION}, Git: commit#=${GIT_COMMIT_COUNT}, branch=${GIT_BRANCH} hash=${GIT_COMMIT_HASH}" )
else()
    message(STATUS "TCAM_VERSION: ${TCAM_VERSION}, Git: No repo found" )
endif()

# All option for configuration come from here
include(CMakeOptions.cmake)

add_compile_options( -Wno-psabi )   # disable ABI change message of g++8
add_compile_options( -Wall -Wextra -pedantic -pthread )

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_compile_options( -fsized-deallocation )
endif()

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_POSITION_INDEPENDENT_CODE ON )

# external/json defaults to the highest cpp standard it can find
# this causes inclusion of std::filesystem
# which is not present on older systems (Ubuntu 18.04 LTS)
# add_definitions is used instead of
# target_compile_definitions as that would require cmake >= 1.12
add_definitions(-DJSON_HAS_CPP_11)

include(CMakeInstall.cmake)

if ( TCAM_ENABLE_CMAKE_CLANGFORMAT_TARGET)
include("clang-format")
endif (TCAM_ENABLE_CMAKE_CLANGFORMAT_TARGET)

if (TCAM_BUILD_UVC_EXTENSION_LOADER_ONLY)


  configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/src/tcam-config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/src/tcam-config.h")

  add_subdirectory(src/v4l2)
  add_subdirectory(tools/tcam-uvc-extension-loader/)
  add_subdirectory(data/uvc-extensions/)
  add_subdirectory(data/udev/)

endif (TCAM_BUILD_UVC_EXTENSION_LOADER_ONLY)

if (TCAM_BUILD_GIGETOOL_ONLY)

  add_subdirectory(external)
  add_subdirectory(src/tcam-network/)
  add_subdirectory(tools/tcam-gigetool)

endif (TCAM_BUILD_GIGETOOL_ONLY)


if (NOT TCAM_EXCLUSIVE_BUILD)

  # TODO: these should not always be included
  add_subdirectory(external)
  add_subdirectory(libs)
  add_subdirectory(src)

  if (TCAM_ENABLE_DATA_INSTALL)
    add_subdirectory(data)
    # TODO: this should not be always on
    install(DIRECTORY examples
      DESTINATION "${CMAKE_INSTALL_PREFIX}/share/theimagingsource/tiscamera/"
      COMPONENT dev)
  endif (TCAM_ENABLE_DATA_INSTALL)


  if (TCAM_BUILD_TOOLS)
    add_subdirectory(tools)
  endif (TCAM_BUILD_TOOLS)

  if (TCAM_BUILD_DOCUMENTATION)
    add_subdirectory(doc)
  endif (TCAM_BUILD_DOCUMENTATION)

  if (TCAM_BUILD_TESTS)
    enable_testing()

    add_subdirectory(tests)
  endif (TCAM_BUILD_TESTS)

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
