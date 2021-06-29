
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
	tcamconvert.h
	tcamconvert.cpp

	gst_caps_helper.h
	gst_caps_helper.cpp
)

target_include_directories( tcamconvert 
PRIVATE 
	"${TCAM_SOURCE_DIR}/src/include/"
	"${TCAM_SOURCE_DIR}/src/lib/dutils_image/include/"
	
PRIVATE
	${GSTREAMER_INCLUDE_DIRS}
	${GSTREAMER_BASE_INCLUDE_DIRS}
	${GSTREAMER_VIDEO_INCLUDE_DIRS}
	${GLIB2_INCLUDE_DIR}
	${GObject_INCLUDE_DIR}
	${INTROSPECTION_INCLUDE_DIR}
)
set_project_warnings(tcamconvert)

target_link_libraries( tcamconvert 

	
PRIVATE
	spdlog::spdlog
	tcamprop
    tcam
	${GOBJECT_LIBRARIES}
	${GLIB2_LIBRARIES}
	${GSTREAMER_LIBRARIES}
	${GSTREAMER_BASE_LIBRARIES}
	${GSTREAMER_VIDEO_LIBRARIES}

	dutils::dutils_img_base
#	dutils::dutils_img_filter_c
	dutils::dutils_img_filter_optimized
)

install(TARGETS tcamconvert
  LIBRARY
  DESTINATION "${TCAM_INSTALL_GST_1_0}"
  COMPONENT tcam-gstreamer-1
)
