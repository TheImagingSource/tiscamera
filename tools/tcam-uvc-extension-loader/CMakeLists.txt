
# Copyright 2018 The Imaging Source Europe GmbH
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



add_executable(tcam-uvc-extension-loader main.cpp)

target_include_directories(tcam-uvc-extension-loader PRIVATE "${TCAM_SOURCE_DIR}/external/CLI11")
target_include_directories(tcam-uvc-extension-loader PRIVATE "${TCAM_SOURCE_DIR}/src/v4l2")


target_link_libraries(tcam-uvc-extension-loader tcam-uvc-extension)
set_project_warnings(tcam-uvc-extension-loader)

install(TARGETS tcam-uvc-extension-loader
  DESTINATION "${TCAM_INSTALL_BIN}"
  COMPONENT bin)
