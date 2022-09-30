
# Copyright 2016 The Imaging Source Europe GmbH
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

find_package(GObject REQUIRED QUIET) # dependency
find_package(GLIB2   REQUIRED QUIET) # dependency
find_package(GIO     REQUIRED QUIET) # dependency

add_executable(tcam-gige-daemon main.cpp CameraListHolder.cpp DaemonClass.cpp)
set_project_warnings(tcam-gige-daemon)

target_include_directories(tcam-gige-daemon PRIVATE "${GIO_INCLUDE_DIR}")
target_include_directories(tcam-gige-daemon PRIVATE ${GOBJECT_INCLUDE_DIR})
target_include_directories(tcam-gige-daemon PRIVATE ${GLIB2_INCLUDE_DIR})

if (TCAM_INTERNAL_ARAVIS)
  # we require two includes
  # as some header are generated
  target_include_directories(tcam-gige-daemon SYSTEM PRIVATE "${TCAM_BINARY_DIR}/external/aravis/src")
  target_include_directories(tcam-gige-daemon SYSTEM PRIVATE "${TCAM_BINARY_DIR}/external/aravis-prefix/src/aravis-build/src/")

  target_link_libraries(tcam-gige-daemon PRIVATE "${ARAVIS_LIB}")

else (TCAM_INTERNAL_ARAVIS)

  target_include_directories(tcam-gige-daemon PRIVATE "${aravis_INCLUDE_DIR}")

  target_link_libraries(tcam-gige-daemon PRIVATE "${aravis_LIBRARIES}")

endif (TCAM_INTERNAL_ARAVIS)

target_link_libraries(tcam-gige-daemon
PRIVATE
    "${LIBXML2_LIBRARIES}"
    "${GIO_LIBRARIES}"
    tcam
    tcam-network
    dl
    outcome::outcome
)

install(TARGETS tcam-gige-daemon
  DESTINATION ${TCAM_INSTALL_BIN}
  COMPONENT bin)
