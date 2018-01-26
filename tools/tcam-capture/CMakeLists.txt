# Copyright 2017 The Imaging Source Europe GmbH
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

find_package(PythonInterp REQUIRED)

if (PYTHONINTERP_FOUND)
  set(TCAM_CAPTURE_PY_INSTALL "${TCAM_INSTALL_LIB}/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages/tcamgigetool")

  install(FILES tcam-capture
    DESTINATION bin
    COMPONENT tcam-capture)

  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/tcam_capture"
    DESTINATION "${TCAM_CAPTURE_PY_INSTALL}"
    COMPONENT tcam-capture)

endif (PYTHONINTERP_FOUND)
