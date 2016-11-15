
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


include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories("${CMAKE_SOURCE_DIR}/src/tcam-network")


set(cflags "-fPIC -w -Wall -Wextra -pedantic -Wundef -Wshadow")
set(lflags "-Wl,--gc-sections -ffunction-sections -fdata-sections -l pthread")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cflags} ")

add_executable(gige-daemon main.cpp CameraListHolder.cpp DaemonClass.cpp)

TARGET_LINK_LIBRARIES(gige-daemon tcam)
TARGET_LINK_LIBRARIES(gige-daemon tcam-network)

install(TARGETS gige-daemon
  DESTINATION ${TCAM_INSTALL_BIN}
  COMPONENT tcam-gige-daemon)
