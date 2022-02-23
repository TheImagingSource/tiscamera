
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

add_library(tcamgstbase STATIC
	tcamgstbase.h
	tcamgstjson.h
	tcamgststrings.h
	tcambinconversion.h

	tcamgstbase.cpp
	tcamgstjson.cpp
	tcamgststrings.cpp
	tcambinconversion.cpp

	spdlog_gst_sink.h
	spdlog_gst_sink.cpp

	version_check.cpp
	version_check.h
)

set_project_warnings(tcamgstbase)

target_include_directories(tcamgstbase
PUBLIC
${GSTREAMER_INCLUDE_DIRS}
)

target_link_libraries(tcamgstbase
  PUBLIC
  tcam
  tcam::tcam-property
  dutils_img::base
  spdlog::spdlog
  tcam::gst-helper-dutils
  )
