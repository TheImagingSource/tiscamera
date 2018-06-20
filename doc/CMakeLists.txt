
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


# add a target to generate API documentation with Doxygen
find_package(Doxygen)

option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})

if(BUILD_DOCUMENTATION)
  if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen is needed to build the documentation.")
  endif()

  set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake)
  set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  configure_file(${doxyfile_in} ${doxyfile} @ONLY)

  # ALL allows us to build the documentation with the project
  add_custom_target(doc ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)

  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)
endif()


if (BUILD_USER_DOC)

  if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen is needed to build the documentation.")
  endif()

  set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile-User.cmake)
  set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  set(doc_dir ${CMAKE_CURRENT_SOURCE_DIR})

  # this needs to be a single string
  # cmake adds ';' between values otherwise
  # thus making it impossible for doxygen to parse the list
  set(doc_file_list
    "${doc_dir}/welcome.md \
    ${doc_dir}/installation.md \
    ${doc_dir}/versioning-and-releases.md \
    ${doc_dir}/gstreamer.md \
    ${doc_dir}/logging.md \
    ${doc_dir}/flags.md \
    ${doc_dir}/properties.md \
    ${doc_dir}/tools.md \
    ${doc_dir}/tcam-capture.md \
    ${doc_dir}/tcam-ctrl.md \
    ${doc_dir}/tcam-gigetool.md \
    ${doc_dir}/camera-ip-conf.md \
    ${doc_dir}/firmware-update.md \
    ${doc_dir}/gige-daemon.md \
    ${doc_dir}/uvcdynctrl.md \
    ${doc_dir}/further-reading.md"
    )

  configure_file(${doxyfile_in} ${doxyfile} @ONLY)

  # ALL allows us to build the documentation with the project
  add_custom_target(user-doc ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/
    COMMENT "Generate User Documentation"
    VERBATIM)

  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)

endif (BUILD_USER_DOC)
