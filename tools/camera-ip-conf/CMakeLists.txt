
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

include_directories(${CMAKE_SOURCE_DIR}/src/tcam-network)

add_executable(camera-ip-conf main.cpp ConsoleManager.cpp)

target_link_libraries(camera-ip-conf tcam-network)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/version.h)

install(TARGETS camera-ip-conf
  DESTINATION bin
  COMPONENT tcam-camera-ip-conf)

if (BUILD_GUI)
  add_subdirectory(gui)
endif (BUILD_GUI)
