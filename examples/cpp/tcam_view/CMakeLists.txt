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

find_package(PkgConfig REQUIRED)

pkg_check_modules(MODULES REQUIRED gstreamer-1.0 gstreamer-app-1.0 gtkmm-3.0)
include_directories(${MODULES_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/common)

# Link the glade file to binary dir so that the executable could be started from the
# build directory
add_custom_command (
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tcam_view.glade
    COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/tcam_view.glade ${CMAKE_CURRENT_BINARY_DIR}/tcam_view.glade
    )

add_executable(tcam_view
                tcam_view.cpp
                property_box.cpp
                logscale.cpp
                main.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/tcam_view.glade)
target_link_libraries(tcam_view tcamgstcamera ${MODULES_LIBRARIES})
