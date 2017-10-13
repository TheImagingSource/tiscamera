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


include(ExternalProject)

if (BUILD_ARAVIS)
  if (TCAM_INTERNAL_ARAVIS)

    if( NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/aravis/.git" )
      message( FATAL_ERROR "The git submodules are not available. Please run:
    git submodule update --init --recursive"
        )
    endif()

    #
    # Get the latest abbreviated commit hash of the working branch
    execute_process(
      COMMAND git log -1 --format=%h
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/aravis"
      OUTPUT_VARIABLE ARAVIS_GIT_COMMIT_HASH_INTERNAL
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )

    set(ARAVIS_GIT_COMMIT_HASH "${ARAVIS_GIT_COMMIT_HASH_INTERNAL}" PARENT_SCOPE)
    set(ARAVIS_VERSION "0.6" PARENT_SCOPE)

    ExternalProject_Add(aravis
      SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/aravis
      CONFIGURE_COMMAND CFLAGS=-fPIC ${CMAKE_CURRENT_SOURCE_DIR}/aravis/autogen.sh --prefix=/usr --enable-usb=no --enable-viewer=no --enable-gtk-doc-html=no --enable-packet-socket=no --enable-gst-plugin=no --enable-gst-0.10-plugin=no
      BUILD_COMMAND make -j4 CFLAGS=-fPIC
      INSTALL_COMMAND ""
      ALWAYS 1)

  else (TCAM_INTERNAL_ARAVIS)

    set(ARAVIS_GIT_COMMIT_HASH "external" PARENT_SCOPE)
    set(ARAVIS_VERSION "unknown" PARENT_SCOPE)

  endif (TCAM_INTERNAL_ARAVIS)

else (BUILD_ARAVIS)

  set(ARAVIS_GIT_COMMIT_HASH "not_configured" PARENT_SCOPE)
  set(ARAVIS_VERSION "not_configured" PARENT_SCOPE)

endif (BUILD_ARAVIS)
