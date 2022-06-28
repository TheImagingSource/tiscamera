
# Copyright 2015 The Imaging Source Europe GmbH
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


set(TCAM_EXTENSION_UNITS usb2.json usb23.json usb33.json usb37.json)

install(FILES ${TCAM_EXTENSION_UNITS}
  DESTINATION ${TCAM_INSTALL_UVC_EXTENSION}
  COMPONENT bin)
