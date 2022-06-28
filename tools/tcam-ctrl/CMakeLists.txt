
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

add_executable(tcam-ctrl
	main.cpp
	general.cpp
	general.h
	properties.h
	properties.cpp
	formats.h
	formats.cpp
	system.h
	system.cpp
)
set_project_warnings(tcam-ctrl)

target_include_directories(tcam-ctrl PRIVATE ${GSTREAMER_INCLUDE_DIRS})
target_include_directories(tcam-ctrl PRIVATE "${TCAM_SOURCE_DIR}/external/CLI11")


target_link_libraries( tcam-ctrl
PRIVATE
	tcam
	tcam::tcam-property

	${GSTREAMER_LIBRARIES}
	${GSTREAMER_BASE_LIBRARIES}
	${GSTREAMER_VIDEO_LIBRARIES}
)

install(TARGETS tcam-ctrl
  DESTINATION ${TCAM_INSTALL_BIN}
  COMPONENT bin)
