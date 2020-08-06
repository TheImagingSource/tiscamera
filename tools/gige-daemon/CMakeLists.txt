
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
include_directories("${GIO_INCLUDE_DIR}")

include_directories(${GOBJECT_INCLUDE_DIR})
include_directories(${GLIB2_INCLUDE_DIR})

include_directories("${TCAM_BINARY_DIR}")
include_directories("${TCAM_SOURCE_DIR}/src")
include_directories("${TCAM_SOURCE_DIR}/src/aravis")

if (TCAM_INTERNAL_ARAVIS)

  # we require two includes
  # as some header are generated
  include_directories(SYSTEM "${TCAM_SOURCE_DIR}/dependencies/aravis/src")
  include_directories(SYSTEM "${TCAM_BINARY_DIR}/dependencies/aravis-prefix/src/aravis-build/src/")


else (TCAM_INTERNAL_ARAVIS)

  include_directories("${aravis_INCLUDE_DIR}")

endif (TCAM_INTERNAL_ARAVIS)

include_directories("${TCAM_SOURCE_DIR}/src")
include_directories("${TCAM_SOURCE_DIR}/src/tcam-network")

add_executable(gige-daemon main.cpp CameraListHolder.cpp DaemonClass.cpp)

TARGET_LINK_LIBRARIES(gige-daemon tcam)
TARGET_LINK_LIBRARIES(gige-daemon tcam-network)
TARGET_LINK_LIBRARIES(gige-daemon "dl")

if (TCAM_INTERNAL_ARAVIS)

  target_link_libraries(gige-daemon "${ARAVIS_LIB}")

else (TCAM_INTERNAL_ARAVIS)

  target_link_libraries(gige-daemon "${aravis_LIBRARIES}")

endif (TCAM_INTERNAL_ARAVIS)

target_link_libraries(gige-daemon "${LIBXML2_LIBRARIES}")
target_link_libraries(gige-daemon "${GIO_LIBRARIES}")
target_link_libraries(gige-daemon "${PCAP_LIBRARIES}")

install(TARGETS gige-daemon
  DESTINATION ${TCAM_INSTALL_BIN}
  COMPONENT tcam-gige-daemon)
