
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

add_subdirectory(dutils_image EXCLUDE_FROM_ALL)
add_subdirectory(gst-helper EXCLUDE_FROM_ALL)

# Set variables to correctly install tcam-property

set(TCAM_PROPERTY_INSTALL_LIB "${TCAM_INSTALL_LIB}")
set(TCAM_PROPERTY_INSTALL_INCLUDE "${TCAM_INSTALL_INCLUDE}")

set(TCAM_PROPERTY_INSTALL_GIR "${TCAM_INSTALL_GIR}")
set(TCAM_PROPERTY_INSTALL_TYPELIB "${TCAM_INSTALL_TYPELIB}")

set(TCAM_PROPERTY_INSTALL_PKGCONFIG "${TCAM_INSTALL_PKGCONFIG}")
set(TCAM_PROPERTY_INSTALL_GST_1_0_HEADER "${TCAM_INSTALL_GST_1_0_HEADER}")

add_subdirectory(tcam-property)
