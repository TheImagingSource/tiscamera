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
add_subdirectory(fmt-7.1.3 EXCLUDE_FROM_ALL)
add_subdirectory(outcome EXCLUDE_FROM_ALL)


include(ExternalProject)

if (TCAM_BUILD_ARAVIS)
  if (TCAM_INTERNAL_ARAVIS)

    if (TCAM_DOWNLOAD_MESON)

      ExternalProject_Add(meson
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/meson
        GIT_REPOSITORY "https://github.com/mesonbuild/meson/"
        GIT_TAG 0.59.0
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""        # Do not install we are statically linking
        UPDATE_COMMAND ""         # Do not rebuild when calling make
        )

      set(TCAM_MESON_CMD ${CMAKE_CURRENT_BINARY_DIR}/meson/meson.py)

    else ()

      find_program(EXE_MESON meson REQUIRED)

      execute_process(COMMAND ${EXE_MESON} "--version"
        OUTPUT_VARIABLE TCAM_MESON_VERSION
        )

      set(TCAM_MESON_MINIMAL_VERSION 0.56.0)

      if (TCAM_MESON_VERSION VERSION_LESS TCAM_MESON_MINIMAL_VERSION)

        message(FATAL_ERROR "aravis requires meson >= ${TCAM_MESON_MINIMAL_VERSION}. Set -DTCAM_DOWNLOAD_MESON=ON to have tiscamera download a more recent version.")

      endif (TCAM_MESON_VERSION VERSION_LESS TCAM_MESON_MINIMAL_VERSION)

      set(TCAM_MESON_CMD meson CACHE INTERNAL "meson build tool executable")
    endif ()

    # meson configuration
    list(APPEND ARAVIS_CONFIG CFLAGS=-fPIC ${TCAM_MESON_CMD} ${CMAKE_CURRENT_BINARY_DIR}/aravis -Dviewer=disabled -Dgst-plugin=disabled -Dfast-heartbeat=false -Ddocumentation=disabled -Dintrospection=disabled -Ddefault_library=static)

    if (TCAM_ARAVIS_USB_VISION)

      list(APPEND ARAVIS_CONFIG "-Dusb=enabled")

    else (TCAM_ARAVIS_USB_VISION)

      list(APPEND ARAVIS_CONFIG "-Dusb=disabled")

    endif (TCAM_ARAVIS_USB_VISION)

    # aravis is cloned during the build phase
    # that means we cannot identify the commit hash
    # during the configure phase
    # set them manually here
    set(ARAVIS_VERSION "0.8" PARENT_SCOPE)
    set(ARAVIS_GIT_COMMIT_HASH "e977fa4" PARENT_SCOPE) # hash of ARAVIS_TAG

    ExternalProject_Add(aravis
      SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/aravis
      GIT_REPOSITORY "https://github.com/AravisProject/aravis.git"
      GIT_TAG 0.8.26
      CONFIGURE_COMMAND ${ARAVIS_CONFIG}
      BUILD_COMMAND ninja
      INSTALL_COMMAND ""        # Do not install we are statically linking
      UPDATE_COMMAND ""         # Do not rebuild when calling make
      )

    if (TCAM_DOWNLOAD_MESON)
      # needed to ensure meson exists before aravis
      add_dependencies(aravis meson)

    endif (TCAM_DOWNLOAD_MESON)

    # cmake integration of generated libraries
    # building with ninja might cause problems otherwise
    set(ARAVIS_LIB "${TCAM_BINARY_DIR}/external/aravis-prefix/src/aravis-build/src/libaravis-0.8.a")

    add_custom_command(OUTPUT "${ARAVIS_LIB}"
      DEPENDS aravis)

    add_custom_target(arv-lib DEPENDS "${ARAVIS_LIB}")

    add_library(aravis-library STATIC IMPORTED GLOBAL)
    add_dependencies(aravis-library arv-lib)
    set_target_properties(aravis-library PROPERTIES
      IMPORTED_LOCATION "${ARAVIS_LIB}"
      )

  else (TCAM_INTERNAL_ARAVIS)

    set(ARAVIS_GIT_COMMIT_HASH "external" PARENT_SCOPE)
    set(ARAVIS_VERSION "unknown" PARENT_SCOPE)
    find_package(Aravis REQUIRED QUIET)
    set(aravis_INCLUDE_DIR "${aravis_INCLUDE_DIR}" CACHE PATH "" )
    set(aravis_LIBRARIES "${aravis_LIBRARIES}" CACHE FILEPATH "")

  endif (TCAM_INTERNAL_ARAVIS)

else (TCAM_BUILD_ARAVIS)

  set(ARAVIS_GIT_COMMIT_HASH "not_configured" PARENT_SCOPE)
  set(ARAVIS_VERSION "not_configured" PARENT_SCOPE)


endif (TCAM_BUILD_ARAVIS)
