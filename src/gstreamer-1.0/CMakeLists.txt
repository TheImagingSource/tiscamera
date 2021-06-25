
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

add_subdirectory( tcamconvert )

set(PLUGINS
  gsttcamsrc
  gsttcambin
  tcamconvert
  )


add_library(tcamgstbase SHARED tcamgstbase.cpp tcamgstjson.cpp tcamgststrings.cpp)

set_project_warnings(tcamgstbase)



target_include_directories(tcamgstbase 
PUBLIC 
	${GLIB2_INCLUDE_DIR}
	${GObject_INCLUDE_DIR}
	${INTROSPECTION_INCLUDE_DIR}
	${GSTREAMER_INCLUDE_DIRS}
	${GSTREAMER_BASE_INCLUDE_DIRS}
	${GSTREAMER_VIDEO_INCLUDE_DIRS}
	${TCAM_SOURCE_DIR}/external/
)

target_link_libraries(tcamgstbase 
PRIVATE
	${GOBJECT_LIBRARIES}
	${GLIB2_LIBRARIES}
	${GSTREAMER_LIBRARIES}
	${GSTREAMER_BASE_LIBRARIES}
	${GSTREAMER_VIDEO_LIBRARIES}
PUBLIC
	tcam
	tcamprop
#	tcam-algorithms
	dutils::dutils_img
	spdlog::spdlog
)

add_library(gsttcamsrc SHARED
	tcamsrc/gsttcamsrc.cpp
	tcamsrc/gsttcammainsrc.cpp
	tcamsrc/mainsrc_tcamprop_impl.cpp
	tcamsrc/mainsrc_device_state.cpp
	tcamsrc/gsttcamdeviceprovider.cpp
	tcamsrc/gsttcamdevice.cpp
	gstmetatcamstatistics.cpp
)


add_library(gsttcambin SHARED gsttcambin.cpp)

target_include_directories( gsttcambin
PRIVATE
)

target_link_libraries( gsttcambin
PRIVATE
	tcamgstbase
)

set_project_warnings(gsttcambin)

target_link_libraries( gsttcamsrc
PRIVATE
	tcamgstbase
)

set_project_warnings(gsttcamsrc)


install(TARGETS tcamgstbase
  LIBRARY
  DESTINATION "${TCAM_INSTALL_LIB}"
  COMPONENT tcam-gstreamer-1)

install(TARGETS ${PLUGINS}
  DESTINATION ${TCAM_INSTALL_GST_1_0}
  COMPONENT tcam-gstreamer-1)

install(FILES gstmetatcamstatistics.h
  DESTINATION "${TCAM_INSTALL_GST_1_0_HEADER}"
  COMPONENT tcam-gstreamer-1)
