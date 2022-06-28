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

set(TCAM_IMAGE_DIR "${TCAM_SOURCE_DIR}/data/images/" CACHE INTERNAL "project folder for images")

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  DESTINATION "${TCAM_INSTALL_IMAGE_DIR}"
  COMPONENT bin
  PATTERN CMakeLists.txt EXCLUDE
  )
