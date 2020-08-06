
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

include_directories("${TCAM_SOURCE_DIR}/src")
include_directories("${TCAM_BINARY_DIR}")

set(srcs
  roi.cpp
  roi_preset.cpp
  roi_change_behavior.cpp
  RegionOfInterest.cpp
  AutoFocus.cpp
  auto_focus.cpp
  image_sampling.cpp
  bayer.cpp
  whitebalance.cpp
  auto_exposure.cpp
  pid_controller.cpp
  biteater.cpp
  work_pool.cpp
  parallel.cpp)

# set(PUBLIC_HEADER tcam.h tcam_c.h)
# TODO: tcam-base shuld link library

add_library(tcam-algorithms SHARED ${srcs} $<TARGET_OBJECTS:tcam-base>)

set_property(TARGET tcam-algorithms PROPERTY VERSION ${TCAM_VERSION})
set_property(TARGET tcam-algorithms PROPERTY SOVERSION ${TCAM_VERSION_MAJOR})

install(FILES ${PUBLIC_HEADER} DESTINATION "${TCAM_INSTALL_INCLUDE}")
install(TARGETS tcam-algorithms LIBRARY DESTINATION "${TCAM_INSTALL_LIB}")
