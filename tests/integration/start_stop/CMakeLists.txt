
# Copyright 2019 The Imaging Source Europe GmbH
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

include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories(${GLIB2_INCLUDE_DIR})
include_directories(${GObject_INCLUDE_DIR})

include_directories(${TCAM_SOURCE_DIR}/external/CLI11)

add_executable(start-stop start-stop.cpp)

target_link_libraries(start-stop ${GSTREAMER_LIBRARIES})
target_link_libraries(start-stop ${GSTREAMER_BASE_LIBRARIES})
target_link_libraries(start-stop ${GSTREAMER_VIDEO_LIBRARIES})
target_link_libraries(start-stop ${GLIB2_LIBRARIES})
target_link_libraries(start-stop ${GOBJECT_LIBRARIES})

configure_file(start-stop-runner.sh "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/start-stop-runner.sh" COPYONLY)
