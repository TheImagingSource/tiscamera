
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
#find_package(GLIB2     REQUIRED QUIET)
#find_package(GObject   REQUIRED QUIET)
#find_package(GObjectIntrospection REQUIRED QUIET)

add_subdirectory(tcamconvert)
add_subdirectory(tcamgstbase)
add_subdirectory(tcamsrc)

add_library(gsttcambin
  SHARED
  gsttcambin.cpp
  tcambin_tcamprop_impl.h
  tcambin_tcamprop_impl.cpp
  tcambin_data.h
  )

target_link_libraries(gsttcambin
  PRIVATE
  tcamgstbase
  tcam::gst-helper
  tcamprop1::provider_gobject
  tcamprop1::consumer
  )

set_project_warnings(gsttcambin)

install(TARGETS gsttcambin
  DESTINATION ${TCAM_INSTALL_GST_1_0}
  COMPONENT tcam-gstreamer-1)
