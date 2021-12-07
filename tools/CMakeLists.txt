
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


add_subdirectory(tcam-ctrl)

if (TCAM_BUILD_WITH_GUI)

  add_subdirectory(tcam-capture)

endif (TCAM_BUILD_WITH_GUI)

if (TCAM_BUILD_V4L2)

  add_subdirectory(tcam-uvc-extension-loader)

endif (TCAM_BUILD_V4L2)

if (TCAM_BUILD_ARAVIS)

  add_subdirectory(tcam-gige-daemon)

  add_subdirectory(tcam-gigetool)

endif (TCAM_BUILD_ARAVIS)
