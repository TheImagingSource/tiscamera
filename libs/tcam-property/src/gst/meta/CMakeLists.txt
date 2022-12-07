
# Copyright 2022 The Imaging Source Europe GmbH
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



add_library(tcamgststatistics SHARED
  gstmetatcamstatistics.cpp
  gstmetatcamstatistics.h
  )

target_include_directories(tcamgststatistics
  PRIVATE
  ${GSTREAMER_INCLUDE_DIRS}
  ${GSTREAMER_BASE_INCLUDE_DIRS}
  ${GSTREAMER_VIDEO_INCLUDE_DIRS}
  PUBLIC ${GLIB2_INCLUDE_DIR}
  )

target_link_libraries( tcamgststatistics
  PRIVATE
  ${GSTREAMER_LIBRARIES}
  ${GSTREAMER_BASE_LIBRARIES}
  ${GSTREAMER_VIDEO_LIBRARIES}
  ${GLIB2_LIBRARIES}
  )


install(TARGETS tcamgststatistics
  DESTINATION ${TCAM_PROPERTY_INSTALL_LIB}
  COMPONENT bin)

install(FILES gstmetatcamstatistics.h
  DESTINATION "${TCAM_PROPERTY_INSTALL_GST_1_0_HEADER}"
  COMPONENT dev)
