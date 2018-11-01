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
set(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS ON)

# this causes problems and makes it impossible to create packages
# without actually installing the files.
# Due to this problem we have to manually track dependencies.
# set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

# string describing all dependencies
# should respect oldest ubuntu LTS
set(debian_dependencies
  "uvcdynctrl, \
gstreamer1.0-tools, \
gstreamer1.0-plugins-good, \
gstreamer1.0-plugins-bad,\
gstreamer1.0-x, \
gstreamer1.0-plugins-base, \
gstreamer1.0-plugins-ugly, \
libxml2, \
libzip4, \
libpcap0.8, \
libaudit1, \
libnotify4, \
python3-gi, \
")


set(CPACK_DEBIAN_PACKAGE_DEPENDS "${debian_dependencies}")

set(CPACK_SET_DESTDIR "on")

# set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_GENERATOR "DEB")

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

if (BUILD_TOOLS)

  # no comma separating CPACK_DEBIAN_PACKAGE_DEPENDS and others as the previous list should end with one
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS} python3-pyqt5 (>= 5.5)")

endif (BUILD_TOOLS)

set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "python-gi (>= 3.20), python3-gi (>= 3.20), python-gst-1.0, python3-gst-1.0")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/deb/postinst.in" "${CMAKE_CURRENT_BINARY_DIR}/deb/postinst")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/deb/prerm.in" "${CMAKE_CURRENT_BINARY_DIR}/deb/prerm")


set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/deb/postinst;${CMAKE_CURRENT_BINARY_DIR}/deb/prerm;")


# The package name is created according to the following rules:
#
# If is is an official release on the master branch:
#     tiscamera_MAJ.MIN.PAT_arch.deb
#
# If it is a preview release:
#     tiscamera_MAJ.MIN.PAT.COMMIT_COUNT~branch_COMMIT_HASH_arch.deb
#
# If a git tag is present it will replace branch and commit hash
#     tiscamera_MAJ.MIN.PAT.COMMIT_COUNT_tag_arch.deb


set(CPACK_PACKAGE_NAME "tiscamera")

string(TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_PACKAGE_NAME_LOWERCASE)
find_program(DPKG_PROGRAM dpkg DOC "dpkg program of Debian-based systems")
if(DPKG_PROGRAM)
  execute_process(
    COMMAND ${DPKG_PROGRAM} --print-architecture
    OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

  # Get commit tag
  execute_process(
    COMMAND git describe --exact-match HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG
    ERROR_VARIABLE GIT_TAG_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

  if (GIT_TAG)
    set(CPACK_PACKAGE_FILE_NAME
      "${CPACK_PACKAGE_NAME_LOWERCASE}_${TCAM_VERSION}_${GIT_TAG}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

  else ()

    if (GIT_COMMIT_HASH)

      set(CPACK_PACKAGE_FILE_NAME
        "${CPACK_PACKAGE_NAME_LOWERCASE}_${TCAM_VERSION}.${GIT_COMMIT_COUNT}~${GIT_BRANCH}_${GIT_COMMIT_HASH}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

    else () # this will be the case when users download the github zip and compile on deb systems

      set(CPACK_PACKAGE_FILE_NAME
        "${CPACK_PACKAGE_NAME_LOWERCASE}_${TCAM_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

    endif(GIT_COMMIT_HASH)
  endif(GIT_TAG)

else(DPKG_PROGRAM)

  set(CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME_LOWERCASE}_${TCAM_VERSION}_${CMAKE_SYSTEM_NAME}")

endif(DPKG_PROGRAM)


set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")


# always last
include(CPack)
