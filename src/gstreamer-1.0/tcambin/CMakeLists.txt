
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

add_library(gsttcambin
  SHARED
  gsttcambin.cpp
  tcambin_tcamprop_impl.h
  tcambin_tcamprop_impl.cpp
  tcambin_data.h
  tcambin_plugin.cpp
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
  COMPONENT bin)
