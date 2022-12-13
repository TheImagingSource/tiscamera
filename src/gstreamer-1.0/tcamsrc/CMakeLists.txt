
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

add_library(gsttcamsrc SHARED
	gsttcamsrc.cpp
	gsttcammainsrc.cpp
	mainsrc_tcamprop_impl.h
	mainsrc_tcamprop_impl.cpp
	mainsrc_device_state.h
	mainsrc_device_state.cpp
    tcamsrc_tcamprop_impl.h
    tcamsrc_tcamprop_impl.cpp

    gsttcambufferpool.h
    gsttcambufferpool.cpp

    mainsrc_gst_device_provider.cpp
	mainsrc_gst_device_provider.h
	mainsrc_gst_device.cpp
	mainsrc_gst_device.h

	tcamsrc_plugin.cpp

	tcambind.h
	tcambind.cpp
    )

  target_include_directories(gsttcamsrc
    PRIVATE
    ${GSTREAMER_INCLUDE_DIRS}
    ${GSTREAMER_BASE_INCLUDE_DIRS}
    ${GSTREAMER_VIDEO_INCLUDE_DIRS}
    )

  target_link_libraries( gsttcamsrc
    PRIVATE
	${GSTREAMER_LIBRARIES}
	${GSTREAMER_BASE_LIBRARIES}
    ${GSTREAMER_VIDEO_LIBRARIES}

	tcamgstbase
	tcam::gst-helper
    tcam::tcam-property
    tcam::tcamgststatistics
    tcamprop1::provider_gobject
    )
set_project_warnings(gsttcamsrc)

install(TARGETS gsttcamsrc
  DESTINATION ${TCAM_INSTALL_GST_1_0}
  COMPONENT bin)
