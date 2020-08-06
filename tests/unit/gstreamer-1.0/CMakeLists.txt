
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


set(test_gst_src
  main.cpp
  find_input_caps.cpp
  find_largest_caps.cpp
  find_input_caps_test_data.h
  index_caps_formats.cpp
  find_camera_src.cpp
  )

include_directories(".")
include_directories("${TCAM_SOURCE_DIR}/dependencies/catch")
include_directories("${TCAM_SOURCE_DIR}/src")
include_directories("${TCAM_SOURCE_DIR}/src/gstreamer-1.0")
include_directories("${TCAM_SOURCE_DIR}/src/gobject")

include_directories("..")


find_package(GStreamer REQUIRED QUIET)
find_package(GLIB2     REQUIRED QUIET)
find_package(GObject   REQUIRED QUIET)
find_package(GObjectIntrospection REQUIRED QUIET)



include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories(${GSTREAMER_BASE_INCLUDE_DIRS})
include_directories(${GSTREAMER_VIDEO_INCLUDE_DIRS})
include_directories(${GLIB2_INCLUDE_DIR})
include_directories(${GObject_INCLUDE_DIR})

add_executable(test-gstreamer-1.0 ${test_gst_src})

target_link_libraries(test-gstreamer-1.0 tcam)
target_link_libraries(test-gstreamer-1.0 tcamgstbase)

target_link_libraries(test-gstreamer-1.0 ${GSTREAMER_LIBRARIES})
target_link_libraries(test-gstreamer-1.0 ${GSTREAMER_BASE_LIBRARIES})
target_link_libraries(test-gstreamer-1.0 ${GSTREAMER_VIDEO_LIBRARIES})
target_link_libraries(test-gstreamer-1.0 ${GOBJECT_LIBRARIES})
target_link_libraries(test-gstreamer-1.0 ${GLIB2_LIBRARIES})
add_test(NAME gstreamer COMMAND test-gstreamer-1.0)



add_executable(find-input-caps-verification find_input_caps_verification.cpp)
# add_test(gstreamer tcam-test-gstreamer-1.0)

target_link_libraries(find-input-caps-verification tcam)
target_link_libraries(find-input-caps-verification tcamgstbase)

target_link_libraries(find-input-caps-verification ${GSTREAMER_LIBRARIES})
target_link_libraries(find-input-caps-verification ${GSTREAMER_BASE_LIBRARIES})
target_link_libraries(find-input-caps-verification ${GSTREAMER_VIDEO_LIBRARIES})
target_link_libraries(find-input-caps-verification ${GOBJECT_LIBRARIES})
target_link_libraries(find-input-caps-verification ${GLIB2_LIBRARIES})
