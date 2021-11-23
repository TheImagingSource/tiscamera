
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



include_directories("${TCAM_SOURCE_DIR}/external/catch")

include_directories("${TCAM_SOURCE_DIR}/src")
include_directories("${TCAM_SOURCE_DIR}/src/gstreamer-1.0")
include_directories("${TCAM_SOURCE_DIR}/src/gobject")

link_libraries( dutils_img::base )

add_subdirectory(integration)

find_package(GStreamer REQUIRED QUIET)
find_package(GLIB2     REQUIRED QUIET)
find_package(GObject   REQUIRED QUIET)
find_package(GObjectIntrospection REQUIRED QUIET)



include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories(${GSTREAMER_BASE_INCLUDE_DIRS})
include_directories(${GSTREAMER_VIDEO_INCLUDE_DIRS})
include_directories(${GLIB2_INCLUDE_DIR})
include_directories(${GObject_INCLUDE_DIR})
