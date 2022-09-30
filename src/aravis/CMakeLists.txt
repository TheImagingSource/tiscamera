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

if (TCAM_BUILD_ARAVIS)

  add_library(tcam-backend-aravis STATIC
    AravisDevice.cpp
    AravisDeviceStream.cpp
    AravisDeviceScaling.cpp
    AravisPropertyBackend.cpp
    AravisDeviceProperties.cpp
    AravisAllocator.h
    AravisAllocator.cpp
    aravis_property_impl.cpp
    aravis_utils.cpp
    aravis_api.cpp
    aravis_api.h
    )

  if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
    target_precompile_headers(tcam-backend-aravis REUSE_FROM tcam-base)
  endif()
  set_project_warnings(tcam-backend-aravis)

  target_link_libraries(tcam-backend-aravis
    PUBLIC
    tcam-base
    )

  find_package(GLIB2   REQUIRED QUIET)
  target_include_directories(tcam-backend-aravis PRIVATE ${GLIB2_INCLUDE_DIR})
  target_link_libraries(tcam-backend-aravis PRIVATE "${GLIB2_LIBRARIES}")

  if (TCAM_INTERNAL_ARAVIS)

    find_package(LibXml2 REQUIRED QUIET)

    find_package(GObject REQUIRED QUIET)
    find_package(GIO     REQUIRED QUIET)


    target_include_directories(tcam-backend-aravis PRIVATE "${GIO_INCLUDE_DIR}")

    target_include_directories(tcam-backend-aravis PRIVATE ${GOBJECT_INCLUDE_DIR})

    target_include_directories(tcam-backend-aravis SYSTEM PRIVATE "${TCAM_BINARY_DIR}/external/aravis/src")
    target_include_directories(tcam-backend-aravis PRIVATE SYSTEM "${TCAM_BINARY_DIR}/external/aravis-prefix/src/aravis-build/src/")

    target_link_libraries(tcam-backend-aravis PRIVATE aravis-library)

    target_link_libraries(tcam-backend-aravis PRIVATE "${LIBXML2_LIBRARIES}")
    target_link_libraries(tcam-backend-aravis PRIVATE "${GIO_LIBRARIES}")

  else (TCAM_INTERNAL_ARAVIS)

    find_package(Aravis REQUIRED QUIET)

    target_include_directories(tcam-backend-aravis PRIVATE SYSTEM ${aravis_INCLUDE_DIR})

    target_link_libraries(tcam-backend-aravis PRIVATE ${aravis_LIBRARIES})

  endif (TCAM_INTERNAL_ARAVIS)

  if (TCAM_ARAVIS_USB_VISION)

    find_package(libusb-1.0 REQUIRED QUIET)
    target_include_directories(tcam-backend-aravis PRIVATE "${LIBUSB_1_INCLUDE_DIRS}")

    target_link_libraries(tcam-backend-aravis PRIVATE "${LIBUSB_1_LIBRARIES}")

  endif (TCAM_ARAVIS_USB_VISION)

endif (TCAM_BUILD_ARAVIS)
