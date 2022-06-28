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


set(TCAM_GIGETOOL_SOURCES
  main.cpp)

add_executable(tcam-gigetool ${TCAM_GIGETOOL_SOURCES})

target_include_directories(tcam-gigetool PRIVATE ${TCAM_SOURCE_DIR}/src/tcam-network)
target_include_directories(tcam-gigetool PRIVATE "${TCAM_SOURCE_DIR}/external/CLI11")

target_link_libraries(tcam-gigetool PRIVATE tcam-network)
set_project_warnings(tcam-gigetool)

install(TARGETS tcam-gigetool
  DESTINATION "${TCAM_INSTALL_BIN}"
  COMPONENT bin)
