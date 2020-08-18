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

set(CPACK_PACKAGE_CHECKSUM "SHA512")

# this causes problems and makes it impossible to create packages
# without actually installing the files.
# Due to this problem we have to manually track dependencies.
# set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

# load dependency list from central point
file(STRINGS "${TCAM_SOURCE_DIR}/dependencies-debian-runtime.txt" debian_dep)

foreach(Line ${debian_dep})
  # Ignore comments and append the rest to a single line.
  if(NOT "${Line}" MATCHES "^#.*$")
    string(REPLACE "\n" "" Line "${Line}")
    set(debian_dependencies "${debian_dependencies}${Line}")
  endif()
endforeach()

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

set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "python-gi (>= 3.20), python-gst-1.0, python3-gst-1.0")

# when dealing with -DBUIL_TOOLS=OFF and similar situations
# the dependency list might end with a trailing `,`
# this will cause dpkg to report an error and abort the installation
# remove it as the list will not be touched after this
string(REGEX REPLACE ",$" "" CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}")

#
# libzip check
# Ubuntu 18.04 has libzip4
# Ubuntu 24.04 has libzip5
# Debian testing (2020.08.18) has libzip4
# check what package exists and install it

find_program(HAS_APT_CACHE apt-cache)

if (HAS_APT_CACHE)
  execute_process(COMMAND apt-cache search --names-only libzip4
    OUTPUT_VARIABLE HAS_LIBZIP4)

  if (HAS_LIBZIP4)
    string(REGEX REPLACE "libzip\\[4-5\\]" "libzip4" CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}")
    message("=====has libzip4")
  else (HAS_LIBZIP4)
    string(REGEX REPLACE "libzip\\[4-5\\]" "libzip5" CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}")
  endif (HAS_LIBZIP4)

else (HAS_APT_CACHE)
  message(WARNING "No apt-cache. Package dependencies may be wrong.")
endif (HAS_APT_CACHE)
# end libzip check


configure_file("${CMAKE_CURRENT_SOURCE_DIR}/deb/postinst.in" "${CMAKE_CURRENT_BINARY_DIR}/deb/postinst")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/deb/prerm.in" "${CMAKE_CURRENT_BINARY_DIR}/deb/prerm")


set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/deb/postinst;${CMAKE_CURRENT_BINARY_DIR}/deb/prerm;")


set(CPACK_PACKAGE_NAME "tiscamera")

include(package-name)

create_package_name(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}" "${TCAM_VERSION}")

set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")


# always last
include(CPack)
