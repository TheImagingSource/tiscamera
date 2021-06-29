
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

add_executable(tcam-ctrl main.cpp general.cpp properties.cpp formats.cpp system.cpp)
set_project_warnings(tcam-ctrl)

target_include_directories(tcam-ctrl PRIVATE ${GSTREAMER_INCLUDE_DIRS})
target_include_directories(tcam-ctrl PRIVATE ${GSTREAMER_BASE_INCLUDE_DIRS})
target_include_directories(tcam-ctrl PRIVATE ${GSTREAMER_VIDEO_INCLUDE_DIRS})
target_include_directories(tcam-ctrl PRIVATE ${GLIB2_INCLUDE_DIR})
target_include_directories(tcam-ctrl PRIVATE ${GObject_INCLUDE_DIR})


target_link_libraries( tcam-ctrl
PRIVATE
	tcamgstbase
	tcam

	${GSTREAMER_LIBRARIES}
	${GSTREAMER_BASE_LIBRARIES}
	${GSTREAMER_VIDEO_LIBRARIES}
	${GOBJECT_LIBRARIES}
	${GLIB2_LIBRARIES}
)

install(TARGETS tcam-ctrl
  DESTINATION ${TCAM_INSTALL_BIN}
  COMPONENT tcam-ctrl)
