
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

add_library(gst_helper STATIC
	"gst_caps_helper.h"
	"gst_caps_helper.cpp"
	"gst_helper.h"
	"gst_ptr.h"

	"gst_element_chain.h"
	"gst_element_chain.cpp"
)

target_include_directories( gst_helper 
PRIVATE
	${GSTREAMER_INCLUDE_DIRS}
	${GLIB2_INCLUDE_DIR}
)
set_project_warnings(gst_helper)

target_link_libraries( gst_helper 
PRIVATE
	spdlog::spdlog
	${GLIB2_LIBRARIES}
	${GSTREAMER_LIBRARIES}

	dutils::dutils_img_base
)
set_property(TARGET gst_helper PROPERTY CXX_VISIBILITY_PRESET hidden)
set_property(TARGET gst_helper PROPERTY VISIBILITY_INLINES_HIDDEN ON)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0") 
    target_precompile_headers( gst_helper PRIVATE
		<vector>
		<string>
		<string_view>
		<algorithm>
		<spdlog/spdlog.h>
	)
endif()

add_library( tcam::gst_helper ALIAS gst_helper )
