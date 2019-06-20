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

find_package(PythonInterp 3 REQUIRED QUIET)

if (PYTHONINTERP_FOUND)

  find_package(PythonModule QUIET) # this quiet makes find_python_module be quiet
  find_python_module(PyQt5 REQUIRED)

  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tcam_capture/image_dir.py.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/tcam_capture/image_dir.py" @ONLY)

  install(PROGRAMS tcam-capture
    DESTINATION bin
    COMPONENT tcam-capture)

  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/tcam_capture"
    DESTINATION "${TCAM_INSTALL_PYTHON3_MODULES}"
    COMPONENT tcam-capture
    PATTERN "__pycache__" EXCLUDE
    PATTERN "image_dir*" EXCLUDE)


  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tcam-capture.desktop.in"
    "${CMAKE_CURRENT_BINARY_DIR}/tcam-capture.desktop" @ONLY)

  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tcam-capture.desktop"
    DESTINATION "${TCAM_INSTALL_DESKTOP_FILES}"
    COMPONENT tcam-capture)

  # this additional step is done to differentiate between a local execution
  # and an installed execution of tcam-capture
  # an installed one should search for globally available images
  # e.g. /usr/share/tiscamera/images
  # whereas a local one should prefer <tiscamera>/data/images
  set(TCAM_IMAGE_DIR "${TCAM_INSTALL_IMAGE_DIR}/images")

  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tcam_capture/image_dir.py.in"
    "${CMAKE_CURRENT_BINARY_DIR}/tcam_capture/image_dir.py" @ONLY)

  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tcam_capture/image_dir.py"
    DESTINATION "${TCAM_INSTALL_PYTHON3_MODULES}/tcam_capture"
    COMPONENT tcam-capture)

endif (PYTHONINTERP_FOUND)
