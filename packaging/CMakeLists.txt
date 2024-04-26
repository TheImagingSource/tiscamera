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

set(CPACK_COMPONENTS_ALL bin dev)
 set(CPACK_DEB_COMPONENT_INSTALL ON)
# set(CPACK_COMPONENTS_ALL filter convolution tegraae )


# cpack_add_component(tcam_bin
#   DISPLAY_NAME "tiscamera binary"
#   DESCRIPTION "tiscamera installation component for binary files"
#   DEPENDS tcam-gige-daemon tcam-gstreamer-1 tcam-capture tcam-introspection gigetool udev)


# set(CPACK_COMPONENTS_ALL tcam-bin tcam-dev)
# set(CPACK_DEB_COMPONENT_INSTALL ON)

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
# tiscamera-dev does not have anythign that requires
set(CPACK_DEBIAN_DEV_PACKAGE_ARCHITECTURE "all")

if (TCAM_BUILD_ARAVIS)

  set(ARAVIS_PRERM "
# Based on output by dh_installsystemd/13.5.2

if [ -z \"${DPKG_ROOT\:-}\" ] && [ \"$1\" = remove ] && [ -d /run/systemd/system ] ; then
	deb-systemd-invoke stop 'tcam-gige-daemon.service' > /dev/null || true
fi
")


  set(ARAVIS_POSTINST "
# End automatically added section
# Based on output by dh_installsystemd/13.5.2

if [ \"$1\" = \"configure\" ] || [ \"$1\" = \"abort-upgrade\" ] || [ \"$1\" = \"abort-deconfigure\" ] || [ \"$1\" = \"abort-remove\" ] ; then

	# was-enabled defaults to true, so new installations run enable.
	if deb-systemd-helper --quiet was-enabled 'tcam-gige-daemon.service'; then
		# Enables the unit on first installation, creates new
		# symlinks on upgrades if the unit file has changed.
		deb-systemd-helper enable 'tcam-gige-daemon.service' >/dev/null || true
	else
		# Update the statefile to add new symlinks (if any), which need to be
		# cleaned up on purge. Also remove old symlinks.
		deb-systemd-helper update-state 'tcam-gige-daemon.service' >/dev/null || true
	fi
fi
if [ \"$1\" = \"configure\" ] || [ \"$1\" = \"abort-upgrade\" ] || [ \"$1\" = \"abort-deconfigure\" ] || [ \"$1\" = \"abort-remove\" ] ; then
	if [ -z \"${DPKG_ROOT\:-}\" ] && [ -d /run/systemd/system ]; then
		systemctl --system daemon-reload >/dev/null || true
		deb-systemd-invoke restart 'tcam-gige-daemon.service' >/dev/null || true
	fi
fi

") #
#  dh_systemd_start --name=tcam-gige-daemon tcam-gige-daemon.service

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

set(TCAM_BIN_DEB_DEPENDECY_CMD ${TCAM_SOURCE_DIR}/scripts/dependency-manager list --package deb -m=${TCAM_PACKAGE_MODULES} --runtime)
set(TCAM_DEV_DEB_DEPENDECY_CMD ${TCAM_SOURCE_DIR}/scripts/dependency-manager list --package deb -m=${TCAM_PACKAGE_MODULES} --compilation)

# if (TCAM_PACKAGE_INCLUDE_BUILD_DEPENDENCIES)
#   list(APPEND TCAM_BIN_DEB_DEPENDECY_CMD --compilation)
# endif (TCAM_PACKAGE_INCLUDE_BUILD_DEPENDENCIES)

execute_process(COMMAND ${TCAM_BIN_DEB_DEPENDECY_CMD}
  WORKING_DIRECTORY "${TCAM_SOURCE_DIR}"
  OUTPUT_VARIABLE CPACK_DEBIAN_TCAM_BIN_PACKAGE_DEPENDS
  RESULT_VARIABLE return_value)

if (return_value EQUAL "1")
  message(WARNING "Unable to parse dependencies. You will not be able to generate packages without errors.")
endif()

execute_process(COMMAND ${TCAM_DEV_DEB_DEPENDECY_CMD}
  WORKING_DIRECTORY "${TCAM_SOURCE_DIR}"
  OUTPUT_VARIABLE CPACK_DEBIAN_TCAM_DEV_PACKAGE_DEPENDS
  RESULT_VARIABLE return_value)

if (return_value EQUAL "1")
  message(WARNING "Unable to parse dependencies. You will not be able to generate packages without errors.")
endif()

# add tiscamera to the tiscamera-dev dependecy list
# this must be done so that the libraries will also be available when linking
set(CPACK_DEBIAN_TCAM_DEV_PACKAGE_DEPENDS "${CPACK_DEBIAN_TCAM_DEV_PACKAGE_DEPENDS}, tiscamera (>=1.1.0)")

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
string(REGEX REPLACE ",$" "" CPACK_DEBIAN_TCAM_BIN_PACKAGE_DEPENDS "${CPACK_DEBIAN_TCAM_BIN_PACKAGE_DEPENDS}")
string(REGEX REPLACE ",$" "" CPACK_DEBIAN_TCAM_DEV_PACKAGE_DEPENDS "${CPACK_DEBIAN_TCAM_DEV_PACKAGE_DEPENDS}")
# remove line breaks
# they may be in the string due to dependency-manager being a commandlione tool
# line breaks may cause the package to not be installable
string(REGEX REPLACE "\n" "" CPACK_DEBIAN_BIN_PACKAGE_DEPENDS "${CPACK_DEBIAN_TCAM_BIN_PACKAGE_DEPENDS}")
string(REGEX REPLACE "\n" "" CPACK_DEBIAN_DEV_PACKAGE_DEPENDS "${CPACK_DEBIAN_TCAM_DEV_PACKAGE_DEPENDS}")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/deb/postinst.in" "${CMAKE_CURRENT_BINARY_DIR}/deb/postinst")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/deb/prerm.in" "${CMAKE_CURRENT_BINARY_DIR}/deb/prerm")

set(CPACK_DEBIAN_BIN_PACKAGE_PROVIDES "tiscamera-tcamproperty (=1.1.0)")
set(CPACK_DEBIAN_BIN_PACKAGE_CONFLICTS "tiscamera-tcamproperty")
set(CPACK_DEBIAN_BIN_PACKAGE_REPLACES "tiscamera-tcamproperty")

set(CPACK_DEBIAN_BIN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/deb/postinst;${CMAKE_CURRENT_BINARY_DIR}/deb/prerm;")


set(CPACK_DEBIAN_BIN_PACKAGE_NAME_BASE "tiscamera")

include(package-name)

set(CPACK_DEBIAN_BIN_PACKAGE_NAME "${CPACK_DEBIAN_BIN_PACKAGE_NAME_BASE}")
create_package_name(CPACK_DEBIAN_BIN_FILE_NAME "${CPACK_DEBIAN_BIN_PACKAGE_NAME_BASE}" "${TCAM_VERSION}")
#create_package_name(CPACK_DEBIAN_BIN_FILE_NAME "${CPACK_DEBIAN_BIN_PACKAGE_NAME}" "${TCAM_VERSION}")
set(CPACK_DEBIAN_BIN_FILE_NAME "${CPACK_DEBIAN_BIN_FILE_NAME}.deb")

git_commit_count(GIT_COMMIT_COUNT)

if (GIT_COMMIT_COUNT)
  set(DEV_VERSION "${TCAM_VERSION}.${GIT_COMMIT_COUNT}")
else ()
  set(DEV_VERSION "${TCAM_VERSION}")
endif ()

set(CPACK_DEBIAN_DEV_PACKAGE_NAME "${CPACK_DEBIAN_BIN_PACKAGE_NAME_BASE}-dev")
create_package_name(CPACK_DEBIAN_DEV_FILE_NAME "${CPACK_DEBIAN_BIN_PACKAGE_NAME_BASE}-dev" "${TCAM_VERSION}")
set(CPACK_DEBIAN_DEV_FILE_NAME "${CPACK_DEBIAN_DEV_FILE_NAME}.deb")

set(CPACK_DEBIAN_BIN_PACKAGE_PRIORITY "optional")


# always last
include(CPack)
