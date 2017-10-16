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

include(CPackComponent)

set(CPACK_COMPONENTS_ALL tiscamera)

#set(CPACK_MONOLITHIC_INSTALL ON)

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "The Imaging Source Europe GmbH <support@theimagingsource.com>")

set(CPACK_PACKAGE_DESCRIPTION "The Imaging Source Linux Software")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Collection of tools and libraries used for camera interaction")
set(CPACK_PACKAGE_VENDOR "The Imaging Source Europe GmbH")
set(CPACK_PACKAGE_CONTACT "support@theimagingsource.com")

set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "uvcdynctrl")
set(CPACK_SET_DESTDIR "on")

# set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_GENERATOR "DEB")

set(CPACK_PACKAGE_DESCRIPTION "The Imaging Source Linux Software Collection")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Collection of tools and libraries used for camera interaction")
set(CPACK_PACKAGE_VENDOR "The Imaging Source Europe GmbH")
set(CPACK_PACKAGE_CONTACT "support@theimagingsource.com")
set(CPACK_PACKAGE_VERSION_MAJOR "${TCAM_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${TCAM_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${TCAM_VERSION_PATCH}")
set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

if (BUILD_ARAVIS)

  set(ARAVIS_PRERM "systemctl stop gige-daemon.service
        systemctl disable gige-daemon.service")
  set(ARAVIS_POSTINST "systemctl enable gige-daemon.service
        systemctl start gige-daemon.service")

else (BUILD_ARAVIS)

  set(ARAVIS_PRERM "")
  set(ARAVIS_POSTINST "")

endif (BUILD_ARAVIS)


configure_file("${CMAKE_CURRENT_SOURCE_DIR}/deb/postinst.in" "${CMAKE_CURRENT_BINARY_DIR}/deb/postinst")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/deb/prerm.in" "${CMAKE_CURRENT_BINARY_DIR}/deb/prerm")


set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/deb/postinst;${CMAKE_CURRENT_BINARY_DIR}/deb/prerm;")

set(CPACK_PACKAGE_NAME "tiscamera")

string(TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_PACKAGE_NAME_LOWERCASE)
find_program(DPKG_PROGRAM dpkg DOC "dpkg program of Debian-based systems")
if(DPKG_PROGRAM)
  execute_process(
    COMMAND ${DPKG_PROGRAM} --print-architecture
    OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  set(CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME_LOWERCASE}_${TCAM_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

else(DPKG_PROGRAM)
  set(CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME_LOWERCASE}_${TCAM_VERSION}_${CMAKE_SYSTEM_NAME}")
endif(DPKG_PROGRAM)


set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")


# always last
include(CPack)
