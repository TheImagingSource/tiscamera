
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

set(NETWORK_SOURCES
  CameraDiscovery.cpp
  Camera.cpp
  Socket.cpp
  NetworkInterface.cpp
  utils.cpp
  FirmwareUpgrade.cpp
  GigE3DevicePortFlashMemory.cpp
  GigE3DevicePortMachXO2.cpp
  GigE3Package.cpp
  GigE3Update.cpp
  FirmwarePackage.cpp
  MachXO2.cpp
  JedecFile.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

add_library(tcam-network SHARED ${NETWORK_SOURCES})

target_link_libraries(tcam-network ${TinyXML_LIBRARIES})
target_link_libraries(tcam-network ${LIBZIP_LIBRARY})

set_property(TARGET tcam-network PROPERTY VERSION ${TCAM_VERSION})
set_property(TARGET tcam-network PROPERTY SOVERSION ${TCAM_VERSION_MAJOR})

install(TARGETS tcam-network
  DESTINATION ${TCAM_INSTALL_LIB}
  COMPONENT tcam-network)
