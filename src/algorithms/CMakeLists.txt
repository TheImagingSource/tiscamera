
# Copyright 2017 The Imaging Source Europe GmbH
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

# set(srcs
#   roi.cpp
#   roi_preset.cpp
#   roi_change_behavior.cpp
#   RegionOfInterest.cpp
#   AutoFocus.cpp
#   auto_focus.cpp
#   image_sampling.cpp
#   bayer.cpp
#   whitebalance.cpp
#   auto_exposure.cpp
#   pid_controller.cpp
#   img_rect_tools.cpp
#   work_pool.cpp
#   parallel.cpp)

set(srcs
  auto_exposure.cpp
  auto_focus.h
  bayer.cpp
  #debayer.h
  image_sampling.h
  parallel.cpp
  pthread_semaphore.h
  roi_change_behavior.cpp
  roi_preset.cpp
  whitebalance.cpp
  auto_exposure.h
  AutoFocus.h
  bayer.h
  image_sampling.cpp
  image_sampling_u8.cpp
  parallel.h
  RegionOfInterest.cpp
  roi_change_behavior.h
  roi_preset.h
  whitebalance.h
  auto_focus.cpp
  auto_sample_image.cpp
  CMakeLists.txt
  image_sampling_float.cpp
  image_sampling_u8.h
  pid_controller.cpp
  RegionOfInterest.h
  auto_alg.cpp
  auto_wb_software_applied_int.cpp
  auto_wb_software_applied_float.cpp
  auto_wb_temperature.cpp
  auto_wb_temperature_sensor_data.cpp
  auto_wb_cam.cpp
  roi.cpp
  std_latch.h
  work_pool.cpp
  AutoFocus.cpp
  auto_sample_image.h
  auto_alg_pass_itf.cpp
  # debayer.cpp
  image_sampling_float.h
  img_rect_tools.cpp
  pid_controller.h
  roi_base.h
  roi.h
  tcam-algorithm.h
  work_pool.h
  )

add_library(tcam-algorithms SHARED ${srcs} $<TARGET_OBJECTS:tcam-base>)
set_project_warnings(tcam-algorithms)

target_include_directories(tcam-algorithms PRIVATE "${TCAM_SOURCE_DIR}/src")
target_include_directories(tcam-algorithms PRIVATE "${TCAM_SOURCE_DIR}/src/include/")
target_include_directories(tcam-algorithms PRIVATE "${TCAM_SOURCE_DIR}/external")
target_include_directories(tcam-algorithms PRIVATE "${TCAM_BINARY_DIR}")

set_property(TARGET tcam-algorithms PROPERTY VERSION ${TCAM_VERSION})
set_property(TARGET tcam-algorithms PROPERTY SOVERSION ${TCAM_VERSION_MAJOR})

# set_target_properties(tcam-algorithms
#   PROPERTIES
#   POSITION_INDEPENDENT_CODE ON
#   CXX_VISIBILITY_PRESET hidden
# )

install(FILES ${PUBLIC_HEADER} DESTINATION "${TCAM_INSTALL_INCLUDE}")
install(TARGETS tcam-algorithms LIBRARY DESTINATION "${TCAM_INSTALL_LIB}")
