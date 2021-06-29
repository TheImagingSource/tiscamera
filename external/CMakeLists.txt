# Copyright 2017 The Imaging Source Europe GmbH
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

add_subdirectory(PugiXml EXCLUDE_FROM_ALL)

add_subdirectory(spdlog EXCLUDE_FROM_ALL)

add_library( outcome INTERFACE )
target_include_directories( outcome INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} )

add_library( outcome::outcome ALIAS outcome )

include(ExternalProject)

if (BUILD_ARAVIS)
  if (TCAM_INTERNAL_ARAVIS)

    # check for intltool
    find_package (Intl REQUIRED QUIET)

    # meson confugration
    # list(APPEND ARAVIS_CONFIG CFLAGS=-fPIC meson ${CMAKE_CURRENT_SOURCE_DIR}/aravis -Dviewer=false -Dgst-plugin=false -Dusb=false -Dfast-heartbeat=true -Ddocumentation=false -Dintrospection=true -Ddefault_library=static)

    # autotools configuration
    list(APPEND ARAVIS_CONFIG CFLAGS=-fPIC ${CMAKE_CURRENT_SOURCE_DIR}/aravis/autogen.sh --prefix=/usr --enable-viewer=no --enable-gtk-doc-html=no --enable-packet-socket=no --enable-gst-plugin=no --enable-gst-0.10-plugin=no --enable-fast-heartbeat --disable-dependency-tracking)

    if (TCAM_ARAVIS_USB_VISION)

      # list(APPEND ARAVIS_CONFIG "-Denable-usb=true")
      list(APPEND ARAVIS_CONFIG "--enable-usb=yes")

    else (TCAM_ARAVIS_USB_VISION)

      # list(APPEND ARAVIS_CONFIG "-Denable-usb=false")
      list(APPEND ARAVIS_CONFIG "--enable-usb=no")

    endif (TCAM_ARAVIS_USB_VISION)

    ExternalProject_Add(aravis
      SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/aravis
      GIT_REPOSITORY "https://github.com/AravisProject/aravis.git"
      GIT_TAG aravis-0-6
      CONFIGURE_COMMAND ${ARAVIS_CONFIG}
      BUILD_COMMAND make -j4 CFLAGS=-fPIC
      INSTALL_COMMAND ""        # Do not install we are statically linking
      UPDATE_COMMAND ""         # Do not rebuild when calling make
      )

    # ExternalProject_Add(aravis
    #   SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/aravis
    #   GIT_REPOSITORY "https://github.com/AravisProject/aravis.git"
    #   GIT_TAG ARAVIS_0_6_3
    #   CONFIGURE_COMMAND ${ARAVIS_CONFIG}
    #   BUILD_COMMAND ninja
    #   INSTALL_COMMAND ""        # Do not install we are statically linking
    #   UPDATE_COMMAND ""         # Do not rebuild when calling make
    #   )

    #
    # Get the latest abbreviated commit hash of the working branch
    execute_process(
      COMMAND git log -1 --format=%h
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/aravis"
      OUTPUT_VARIABLE ARAVIS_GIT_COMMIT_HASH_INTERNAL
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )

    set(ARAVIS_GIT_COMMIT_HASH "${ARAVIS_GIT_COMMIT_HASH_INTERNAL}" PARENT_SCOPE)
    set(ARAVIS_VERSION "0.7" PARENT_SCOPE)

  else (TCAM_INTERNAL_ARAVIS)

    set(ARAVIS_GIT_COMMIT_HASH "external" PARENT_SCOPE)
    set(ARAVIS_VERSION "unknown" PARENT_SCOPE)
    find_package(Aravis_0_8 REQUIRED QUIET)
    set(aravis_INCLUDE_DIR "${aravis_0_8_INCLUDE_DIR}" CACHE PATH "" )
    set(aravis_LIBRARIES "${aravis_0_8_LIBRARY}" CACHE FILEPATH "")

  endif (TCAM_INTERNAL_ARAVIS)

else (BUILD_ARAVIS)

  set(ARAVIS_GIT_COMMIT_HASH "not_configured" PARENT_SCOPE)
  set(ARAVIS_VERSION "not_configured" PARENT_SCOPE)


endif (BUILD_ARAVIS)
