
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


find_package(GStreamer REQUIRED QUIET)
find_package(GLIB2     REQUIRED QUIET)
find_package(GObject   REQUIRED QUIET)
find_package(GObjectIntrospection REQUIRED QUIET)

include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories(${GSTREAMER_BASE_INCLUDE_DIRS})
include_directories(${GSTREAMER_VIDEO_INCLUDE_DIRS})
include_directories(${GLIB2_INCLUDE_DIR})
include_directories(${GObject_INCLUDE_DIR})


include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/dependencies")
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/src/gstreamer-1.0")
include_directories("${CMAKE_SOURCE_DIR}/src")

add_executable(tcam-ctrl main.cpp properties.cpp formats.cpp)
target_link_libraries(tcam-ctrl tcam)
target_link_libraries(tcam-ctrl tcamgstbase)


target_link_libraries(tcam-ctrl ${GSTREAMER_LIBRARIES})
target_link_libraries(tcam-ctrl ${GSTREAMER_BASE_LIBRARIES})
target_link_libraries(tcam-ctrl ${GSTREAMER_VIDEO_LIBRARIES})
target_link_libraries(tcam-ctrl ${GOBJECT_LIBRARIES})
target_link_libraries(tcam-ctrl ${GLIB2_LIBRARIES})

install(TARGETS tcam-ctrl
  DESTINATION ${TCAM_INSTALL_BIN}
  COMPONENT tcam-ctrl)
