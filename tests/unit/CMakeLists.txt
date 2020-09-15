
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

if (BUILD_GST_1_0)
  add_subdirectory(gstreamer-1.0)
endif (BUILD_GST_1_0)

if (BUILD_ARAVIS)
  add_subdirectory(tcam-network)
endif (BUILD_ARAVIS)

include_directories("${TCAM_SOURCE_DIR}/external/catch")


set(test-main-src
  main.cpp
  videoformat_test.cpp)

add_executable(test-main-lib ${test-main-src})

target_link_libraries(test-main-lib tcam)

add_test(main-lib test-main-lib)
