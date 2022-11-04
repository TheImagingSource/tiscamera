
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

add_subdirectory(images)

if (TCAM_BUILD_TOOLS)
  add_subdirectory(bash-completion)
endif (TCAM_BUILD_TOOLS)

if (TCAM_BUILD_ARAVIS AND TCAM_BUILD_TOOLS)

  # leave as a reminder on how to handle aravis versions should a
  # more internal pkg-config file be needed

  # find_package(Aravis REQUIRED)
  # string(FIND ${aravis_LIBRARY} "0.6" aravis_VERSION)
  # if (${aravis_VERSION} MATCHES "-1")
  #   set(tcam_pkgconfig_dependencies "${tcam_pkgconfig_dependencies} aravis-0.4")
  # else()
  #   set(tcam_pkgconfig_dependencies "${tcam_pkgconfig_dependencies} aravis-0.6")
  # endif (${aravis_VERSION} MATCHES "-1")

  add_subdirectory(systemd)
endif (TCAM_BUILD_ARAVIS AND TCAM_BUILD_TOOLS)


if (TCAM_BUILD_V4L2)

  # set(tcam_pkgconfig_dependencies "${tcam_pkgconfig_dependencies} libudev libusb-1.0")

  add_subdirectory(uvc-extensions)

endif (TCAM_BUILD_V4L2)

if (TCAM_BUILD_V4L2 OR TCAM_BUILD_LIBUSB OR TCAM_ARAVIS_USB_VISION)
  # udev rules are needed for device permissions
  # libusb and usb3vision require access to change the used driver

  add_subdirectory(udev)

endif (TCAM_BUILD_V4L2 OR TCAM_BUILD_LIBUSB OR TCAM_ARAVIS_USB_VISION)

if (NOT TCAM_EXCLUSIVE_BUILD)

  # define external dependencies for linkage
  # gobject is needed for tcam-property
  # since the Requires field contains tcam-property let that take care of gobject
  #set(tcam_pkgconfig_dependencies "gobject-introspection-1.0")


  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/tcam.pc.cmake  ${CMAKE_CURRENT_BINARY_DIR}/tcam.pc @ONLY)

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tcam.pc DESTINATION ${TCAM_INSTALL_PKGCONFIG} COMPONENT dev)

endif (NOT TCAM_EXCLUSIVE_BUILD)
