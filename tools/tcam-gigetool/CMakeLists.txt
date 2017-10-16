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

# Version number for tcam_gigetool
set(TCAM_GIGETOOL_VERSION "1.0" STRING "Version number for tcam_gigetool")

include_directories(${CMAKE_SOURCE_DIR}/src/tcam-network)

add_library(tcam_gigewrapper SHARED gige_wrapper.cpp)

target_link_libraries(tcam_gigewrapper tcam-network)

install(TARGETS tcam_gigewrapper
  DESTINATION lib
  COMPONENT tcam-gigetool)

find_program(PYTHON "python")

if (PYTHON)
    set(SETUP_PY_IN   "${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in")
    set(SETUP_PY      "${CMAKE_CURRENT_BINARY_DIR}/setup.py")
    set(VERSION_PY_IN "${CMAKE_CURRENT_SOURCE_DIR}/tcamgigetool/version.py.in")
    set(VERSION_PY    "${CMAKE_CURRENT_SOURCE_DIR}/tcamgigetool/version.py")
    set(DEPS          "${CMAKE_CURRENT_SOURCE_DIR}/tcamgigetool/tcam_gigetool.py")
    set(OUTPUT        "${CMAKE_CURRENT_BINARY_DIR}/build/timestamp")

    configure_file(${SETUP_PY_IN} ${SETUP_PY})
    configure_file(${VERSION_PY_IN} ${VERSION_PY})

    add_custom_command(OUTPUT ${OUTPUT}
                       COMMAND ${PYTHON} ${SETUP_PY} build
                       COMMAND ${CMAKE_COMMAND} -E touch ${OUTPUT}
                       DEPENDS ${DEPS})

    add_custom_target(target ALL DEPENDS ${OUTPUT})

    # install(CODE "execute_process(COMMAND ${PYTHON} ${SETUP_PY} install)"
            # COMPONENT tcam-gigetool)
endif()
