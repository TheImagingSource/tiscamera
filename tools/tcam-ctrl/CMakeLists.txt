
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

include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/src/gstreamer-1.0")
include_directories("${CMAKE_SOURCE_DIR}/src")

add_executable(tcam-ctrl main.cpp properties.cpp formats.cpp multimedia.cpp)
target_link_libraries(tcam-ctrl tcam)

install(TARGETS tcam-ctrl
  DESTINATION ${TCAM_INSTALL_BIN}
  COMPONENT tcam-ctrl)
