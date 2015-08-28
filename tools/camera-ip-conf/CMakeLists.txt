
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

find_package(TinyXML REQUIRED)
include_directories(${TinyXML_INCLUDE_DIRS})

find_package(LibZip REQUIRED)
include_directories(${LIBZIP_INCLUDE_DIR})


add_library(ip-conf-base OBJECT CameraDiscovery.cpp Camera.cpp Socket.cpp NetworkInterface.cpp utils.cpp FirmwareUpgrade.cpp)

add_executable(camera-ip-conf-cli main.cpp ConsoleManager.cpp $<TARGET_OBJECTS:ip-conf-base> )

target_link_libraries(camera-ip-conf-cli ${TinyXML_LIBRARIES})
target_link_libraries(camera-ip-conf-cli ${LIBZIP_LIBRARY})

install(TARGETS camera-ip-conf-cli DESTINATION bin)

if (BUILD_GUI)
  add_subdirectory(gui)
endif (BUILD_GUI)
