
# Copyright 2016 The Imaging Source Europe GmbH
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


if(BUILD_ARAVIS)

  configure_file(gige-daemon.service.cmake ${CMAKE_CURRENT_BINARY_DIR}/gige-daemon.service)

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/gige-daemon.service
    DESTINATION ${TCAM_INSTALL_SYSTEMD}
    COMPONENT tcam-gige-daemon)

endif(BUILD_ARAVIS)
