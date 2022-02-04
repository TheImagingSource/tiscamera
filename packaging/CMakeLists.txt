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


if (TCAM_EXCLUSIVE_BUILD)
  set(TCAM_PACKAGE_MODULES "")
else (TCAM_EXCLUSIVE_BUILD)
  set(TCAM_PACKAGE_MODULES "base,gstreamer")
endif (TCAM_EXCLUSIVE_BUILD)


function(add_deps dep)
  if (NOT "${TCAM_PACKAGE_MODULES}" STREQUAL "")
    set(TCAM_PACKAGE_MODULES "${TCAM_PACKAGE_MODULES},")
  endif (NOT "${TCAM_PACKAGE_MODULES}" STREQUAL "")

  set(TCAM_PACKAGE_MODULES "${TCAM_PACKAGE_MODULES}${dep}" PARENT_SCOPE)

endfunction()


set(CPACK_SET_DESTDIR "on")

# set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_GENERATOR "DEB")

set(CPACK_PACKAGE_VERSION_MAJOR "${TCAM_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${TCAM_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${TCAM_VERSION_PATCH}")

git_commit_count(GIT_COMMIT_COUNT)

set(CPACK_DEBIAN_PACKAGE_VERSION "${TCAM_VERSION}.${GIT_COMMIT_COUNT}")
set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

if (TCAM_BUILD_ARAVIS)

  set(ARAVIS_PRERM "systemctl stop gige-daemon.service
        systemctl disable gige-daemon.service")
  set(ARAVIS_POSTINST "systemctl enable gige-daemon.service
        systemctl start gige-daemon.service")

  add_deps("aravis")

else (TCAM_BUILD_ARAVIS)

  set(ARAVIS_PRERM "")
  set(ARAVIS_POSTINST "")

endif (TCAM_BUILD_ARAVIS)

if (TCAM_BUILD_TOOLS)

  add_deps("tools")

  if (TCAM_BUILD_WITH_GUI)

    add_deps("gui")

  endif (TCAM_BUILD_WITH_GUI)

endif (TCAM_BUILD_TOOLS)

if (TCAM_BUILD_DOCUMENTATION)
  add_deps("documentation")
endif (TCAM_BUILD_DOCUMENTATION)

set(TCAM_DEB_DEPENDECY_CMD ${TCAM_SOURCE_DIR}/scripts/dependency-manager list --package deb -m=${TCAM_PACKAGE_MODULES})

if (TCAM_PACKAGE_INCLUDE_BUILD_DEPENDENCIES)
  list(APPEND TCAM_DEB_DEPENDECY_CMD --compilation)
endif (TCAM_PACKAGE_INCLUDE_BUILD_DEPENDENCIES)

execute_process(COMMAND ${TCAM_DEB_DEPENDECY_CMD}
  WORKING_DIRECTORY "${TCAM_SOURCE_DIR}"
  OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_DEPENDS
  RESULT_VARIABLE return_value)

if (return_value EQUAL "1")
  message(WARNING "Unable to parse dependencies. You will not be able to generate packages without errors.")
endif()

if (NOT TCAM_DISTRIBUTION_DESCRIPTION)
  # TCAM_DISTRIBUTION_DESCRIPTION will be read be package-name.cmake
  execute_process(COMMAND ${TCAM_SOURCE_DIR}/scripts/dependency-manager dist-release
    WORKING_DIRECTORY "${TCAM_SOURCE_DIR}"
    OUTPUT_VARIABLE TCAM_DISTRIBUTION_DESCRIPTION
    RESULT_VARIABLE return_value)

  if (return_value EQUAL "1")
    message(WARNING "Unable to parse distribution. Packages will not contain distribution.")
  endif()
endif (NOT TCAM_DISTRIBUTION_DESCRIPTION)


### required for tcam-capture

# when dealing with -DBUIL_TOOLS=OFF and similar situations
# the dependency list might end with a trailing `,`
# this will cause dpkg to report an error and abort the installation
# remove it as the list will not be touched after this
string(REGEX REPLACE ",$" "" CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}")
# remove line breaks
# they may be in the string due to dependency-manager being a commandlione tool
# line breaks may cause the package to not be installable
string(REGEX REPLACE "\n" "" CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/deb/postinst.in" "${CMAKE_CURRENT_BINARY_DIR}/deb/postinst")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/deb/prerm.in" "${CMAKE_CURRENT_BINARY_DIR}/deb/prerm")
configure_file("${CMAKE_CURRENT_BINARY_DIR}/../data/systemd/tcam-gige-daemon.service" "${CMAKE_CURRENT_BINARY_DIR}/deb/tiscamera.tcam-gige-daemon.service")

set(CPACK_DEBIAN_PACKAGE_PROVIDES "tiscamera-tcamproperty")
set(CPACK_DEBIAN_PACKAGE_CONFLICTS "tiscamera-tcamproperty")
set(CPACK_DEBIAN_PACKAGE_REPLACES "tiscamera-tcamproperty")

set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/deb/postinst;${CMAKE_CURRENT_BINARY_DIR}/deb/prerm;")


set(CPACK_PACKAGE_NAME "tiscamera")

include(package-name)

create_package_name(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}" "${TCAM_VERSION}")

set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")


# always last
include(CPack)
