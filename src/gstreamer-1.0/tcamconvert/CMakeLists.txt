
# Copyright 2021 The Imaging Source Europe GmbH
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

add_library(tcamconvert SHARED
  "tcamconvert.h"
  "tcamconvert.cpp"



  "tcamconvert_context.h"
  "tcamconvert_context.cpp"
  "transform_impl.h"
  "transform_impl.cpp"
  )

target_include_directories(tcamconvert
  PRIVATE
  ${GSTREAMER_INCLUDE_DIRS}
  ${GSTREAMER_BASE_INCLUDE_DIRS}
  ${GSTREAMER_VIDEO_INCLUDE_DIRS}
  )

set_project_warnings(tcamconvert)

target_link_libraries(tcamconvert
  PRIVATE
  spdlog::spdlog
  tcam::gst-helper-dutils
  tcam::tcam-property
  tcam
  ${GSTREAMER_LIBRARIES}
  ${GSTREAMER_BASE_LIBRARIES}
  ${GSTREAMER_VIDEO_LIBRARIES}

  dutils_img::base
  dutils_img::img_filter_optimized

  tcamprop1::consumer
  )

set_property(TARGET tcamconvert PROPERTY CXX_VISIBILITY_PRESET hidden)
set_property(TARGET tcamconvert PROPERTY VISIBILITY_INLINES_HIDDEN ON)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
  target_precompile_headers(tcamconvert
    PRIVATE
	<vector>
	<string>
	<string_view>
	<algorithm>
	<outcome/result.hpp>
	"../../logging.h"		# Include spdlog indirectly
	)
endif()


install(TARGETS tcamconvert
  LIBRARY
  DESTINATION "${TCAM_INSTALL_GST_1_0}"
  COMPONENT bin
  )
