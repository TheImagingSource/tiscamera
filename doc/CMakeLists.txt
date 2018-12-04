
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

set(doxygen_files "")
set(doxyfile_layout "")

if (BUILD_USER_DOC)

  set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile-User.cmake)
  set(doxyfile_layout ${CMAKE_CURRENT_SOURCE_DIR}/doxygen-layout-user.xml)
  set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  set(doc_dir "${CMAKE_CURRENT_SOURCE_DIR}/pages")

  # this needs to be a single string
  # cmake adds ';' between values otherwise
  # thus making it impossible for doxygen to parse the list
  #
  # The order of this list is also the order of the side pane in doxygen!
  #
  set(doc_file_list
    "${doc_dir}/welcome.md \
    ${doc_dir}/building.md \
    ${doc_dir}/installation.md \
    ${doc_dir}/versioning-and-releases.md \
    ${doc_dir}/documentation.md \
    ${doc_dir}/gstreamer.md \
    ${doc_dir}/logging.md \
    ${doc_dir}/flags.md \
    ${doc_dir}/gobject-introspection.md \
    ${doc_dir}/properties.md \
    ${doc_dir}/tools.md \
    ${doc_dir}/tcam-capture.md \
    ${doc_dir}/tcam-ctrl.md \
    ${doc_dir}/trigger.md \
    ${doc_dir}/examples.md \
    ${doc_dir}/tcam-gigetool.md \
    ${doc_dir}/camera-ip-conf.md \
    ${doc_dir}/firmware-update.md \
    ${doc_dir}/gige-daemon.md \
    ${doc_dir}/uvcdynctrl.md \
    ${doc_dir}/tcam-uvc-extension-loader.md \
    ${doc_dir}/udev-rules.md \
    ${doc_dir}/optimizations.md \
    ${doc_dir}/timestamps.md \
    ${doc_dir}/packaging.md \
    ${doc_dir}/licensing.md \
    ${doc_dir}/further-reading.md \
    ${doc_dir}/contact.md \
    "
    )

  set(doxygen_files "${doxygen_files} ${doc_file_list}")

  set(BUILD_DOC TRUE)

endif (BUILD_USER_DOC)


if(BUILD_DOCUMENTATION)

  set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake)
  set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  set(doxygen_files "${doxygen_files} \
${CMAKE_SOURCE_DIR}/src/ \
${CMAKE_SOURCE_DIR}/tools/ \
")

  set(BUILD_DOC TRUE)

endif(BUILD_DOCUMENTATION)


if ((BUILD_USER_DOC) AND (BUILD_DOCUMENTATION))

  set(doxyfile_layout ${CMAKE_CURRENT_SOURCE_DIR}/DoxygenLayout.xml)

  set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile-ALL.cmake)
  set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

endif ((BUILD_USER_DOC) AND (BUILD_DOCUMENTATION))



if (BUILD_DOC)

  if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen is needed to build the documentation.")
  endif()

  configure_file(${doxyfile_in} ${doxyfile} @ONLY)

  # ALL allows us to build the documentation with the project
  add_custom_target(doc ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/
    COMMENT "Generate User Documentation"
    VERBATIM)

  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION "${TCAM_INSTALL_DOCUMENTATION}")

  # This adds the doxygen output directories to the 'make clean' command
  set_property(DIRECTORY APPEND PROPERTY
    ADDITIONAL_MAKE_CLEAN_FILES
    "${CMAKE_CURRENT_BINARY_DIR}/latex"
    "${CMAKE_CURRENT_BINARY_DIR}/html"
    "${CMAKE_CURRENT_BINARY_DIR}/man")

endif (BUILD_DOC)
