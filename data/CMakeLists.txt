
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

# define external dependencies for linkage
set(tcam_pkgconfig_dependencies "tinyxml")

if (BUILD_ARAVIS)
  set(tcam_pkgconfig_dependencies "${tcam_pkgconfig_dependencies} aravis-0.4")

  add_subdirectory(systemd)
endif (BUILD_ARAVIS)

if (BUILD_V4L2)
  set(tcam_pkgconfig_dependencies "${tcam_pkgconfig_dependencies} libudev")

  add_subdirectory(uvc-extensions)

  add_subdirectory(udev)

endif (BUILD_V4L2)

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/tcam.pc.cmake  ${CMAKE_CURRENT_BINARY_DIR}/tcam.pc @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tcam.pc DESTINATION lib/pkgconfig)
