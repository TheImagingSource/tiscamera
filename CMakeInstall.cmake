
# Copyright 2020 The Imaging Source Europe GmbH
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

# This file contains all installation paths of tiscamera

# helper funcion to receive the value of an arbitrary variable
function(pkg_check_variable _pkg _name)
  string(TOUPPER ${_pkg} _pkg_upper)
  string(TOUPPER ${_name} _name_upper)
  string(REPLACE "-" "_" _pkg_upper ${_pkg_upper})
  string(REPLACE "-" "_" _name_upper ${_name_upper})
  set(_output_name "${_pkg_upper}_${_name_upper}")

  execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=${_name} ${_pkg}
    OUTPUT_VARIABLE _pkg_result
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  set("${_output_name}" "${_pkg_result}" CACHE STRING "pkg-config variable ${_name} of ${_pkg}")
endfunction()

find_program(EXECUTABLE_DPKG_ARCH dpkg-architecture
  DOC "dpkg-architecture program of Debian-based systems")

if (EXECUTABLE_DPKG_ARCH)
  execute_process(COMMAND ${EXECUTABLE_DPKG_ARCH} -qDEB_HOST_MULTIARCH
    OUTPUT_VARIABLE DEB_HOST_MULTIARCH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  string(REGEX REPLACE "\n$" "" DEB_HOST_MULTIARCH "${DEB_HOST_MULTIARCH}")

endif (EXECUTABLE_DPKG_ARCH)


if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

  set(TCAM_INSTALL_LIB "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "library installation path")
  set(TCAM_INSTALL_INCLUDE "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "header installation path")
  set(TCAM_INSTALL_BIN "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "binary installation path")
  set(TCAM_INSTALL_UDEV "/lib/udev/rules.d" CACHE PATH "udev rules installation path")
  set(TCAM_INSTALL_SYSTEMD "/lib/systemd/system/" CACHE PATH "systemd unit installation path")

  if (DEB_HOST_MULTIARCH)
    set(TCAM_INSTALL_PKGCONFIG "${CMAKE_INSTALL_PREFIX}/lib/${DEB_HOST_MULTIARCH}/pkgconfig"
      CACHE PATH "pkgconfig installation path")
  else (DEB_HOST_MULTIARCH)
    set(TCAM_INSTALL_PKGCONFIG "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig"
      CACHE PATH "pkgconfig installation path")
  endif (DEB_HOST_MULTIARCH)

  set(TCAM_INSTALL_DOCUMENTATION
    "${CMAKE_INSTALL_PREFIX}/share/theimagingsource/tiscamera/doc"
    CACHE PATH "documentation installation path")
  set(TCAM_INSTALL_UVC_EXTENSION
    "${CMAKE_INSTALL_PREFIX}/share/theimagingsource/tiscamera/uvc-extension/"
    CACHE PATH "Folder for uvc extension descriptions")
  set(TCAM_INSTALL_DESKTOP_FILES "${CMAKE_INSTALL_PREFIX}/share/applications" CACHE PATH "Folder for storing .desktop files")

else()

  set(TCAM_INSTALL_LIB "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "library installation path" FORCE)
  set(TCAM_INSTALL_INCLUDE "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "header installation path" FORCE)
  set(TCAM_INSTALL_BIN "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "binary installation path" FORCE)
  set(TCAM_INSTALL_UDEV "/etc/udev/rules.d" CACHE PATH "udev rules installation path" FORCE)
  set(TCAM_INSTALL_SYSTEMD "/lib/systemd/system/" CACHE PATH "systemd unit installation path")

  if (DEB_HOST_MULTIARCH)
    set(TCAM_INSTALL_PKGCONFIG "${CMAKE_INSTALL_PREFIX}/lib/${DEB_HOST_MULTIARCH}/pkgconfig"
      CACHE PATH "pkgconfig installation path")
  else (DEB_HOST_MULTIARCH)
    set(TCAM_INSTALL_PKGCONFIG "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig"
      CACHE PATH "pkgconfig installation path")
  endif (DEB_HOST_MULTIARCH)

  set(TCAM_INSTALL_DOCUMENTATION
    "${CMAKE_INSTALL_PREFIX}/share/theimagingsource/tiscamera/doc"
    CACHE PATH "documentation installation path" FORCE)
  set(TCAM_INSTALL_UVC_EXTENSION
    "${CMAKE_INSTALL_PREFIX}/share/theimagingsource/tiscamera/uvc-extension/"
    CACHE PATH "Folder for uvc extension descriptions" FORCE)
  set(TCAM_INSTALL_DESKTOP_FILES "${CMAKE_INSTALL_PREFIX}/share/applications" CACHE PATH "Folder for storing .desktop files" FORCE)


endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

if (BUILD_TOOLS)

  find_package(PythonInterp 3 REQUIRED QUIET)

  execute_process(COMMAND
    python3 -c "from distutils.sysconfig import get_python_lib; lb=get_python_lib(); print(lb.startswith('${CMAKE_INSTALL_PREFIX}') and lb or lb)"
    OUTPUT_VARIABLE
    PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)

  set(TCAM_INSTALL_PYTHON3_MODULES "${PYTHON_SITE_PACKAGES}" CACHE PATH "Installation path for python3 modules")

endif (BUILD_TOOLS)


##
# GObject installation directories
##

# check for better suited install directories
pkg_check_variable(gobject-introspection-1.0 girdir)
pkg_check_variable(gobject-introspection-1.0 typelibdir)

if (GOBJECT_INTROSPECTION_1.0_GIRDIR)
  set(TCAM_INSTALL_GIR "${GOBJECT_INTROSPECTION_1.0_GIRDIR}"
    CACHE PATH "gobject introspection installation path")
else()
  set(TCAM_INSTALL_GIR "${CMAKE_INSTALL_PREFIX}/share/gir-1.0/"
    CACHE PATH "gobject introspection installation path")
endif()

if (GOBJECT_INTROSPECTION_1.0_TYPELIBDIR)
  set(TCAM_INSTALL_TYPELIB "${GOBJECT_INTROSPECTION_1.0_TYPELIBDIR}"
    CACHE PATH "gobject introspection typelib installation path")
else()
  set(TCAM_INSTALL_TYPELIB "${CMAKE_INSTALL_PREFIX}/lib/girepository-1.0"
    CACHE PATH "gobject introspection typelib installation path")
endif()


##
# GStreamer installation directories
##
if (BUILD_GST_1_0)

  pkg_check_variable(gstreamer-1.0 pluginsdir)
  pkg_check_variable(gstreamer-1.0 includedir)


  if (GSTREAMER_1.0_PLUGINSDIR)
    set(TCAM_INSTALL_GST_1_0 "${GSTREAMER_1.0_PLUGINSDIR}"
      CACHE PATH "gstreamer-1.0 module installation path")
  else()
    set(TCAM_INSTALL_GST_1_0 "${CMAKE_INSTALL_PREFIX}/lib/gstreamer-1.0"
      CACHE PATH "gstreamer-1.0 module installation path")
  endif()

  if (GSTREAMER_1.0_INCLUDEDIR)
    set(TCAM_INSTALL_GST_1_0_HEADER "${GSTREAMER_1.0_INCLUDEDIR}"
      CACHE PATH "gstreamer-1.0 header installation path")
  else()
    set(TCAM_INSTALL_GST_1_0_HEADER "${CMAKE_INSTALL_PREFIX}/include/gstreamer-1.0"
      CACHE PATH "gstreamer-1.0 header installation path")
  endif (GSTREAMER_1.0_INCLUDEDIR)

endif (BUILD_GST_1_0)



function(print_install_overview)

  MESSAGE(STATUS "")

  if (NOT TCAM_EXCLUSIVE_BUILD)

    MESSAGE(STATUS "Build gstreamer-1.0 plugins:   " ${BUILD_GST_1_0})
    MESSAGE(STATUS "Support for GigE via aravis:   " ${BUILD_ARAVIS})
    MESSAGE(STATUS "Support for USB cameras:       " ${BUILD_V4L2})
    MESSAGE(STATUS "Support for LibUsb cameras:    " ${BUILD_LIBUSB})
    MESSAGE(STATUS "Build additional utilities:    " ${BUILD_TOOLS})
    MESSAGE(STATUS "Build documentation            " ${BUILD_DOCUMENTATION})
    MESSAGE(STATUS "Build tests                    " ${BUILD_TESTS})
    MESSAGE(STATUS "")

  endif (NOT TCAM_EXCLUSIVE_BUILD)

  if (TCAM_BUILD_FIRMWARE_UPDATE_ONLY)
    MESSAGE(STATUS " !! Only building firmware-update and associated files !!")
  endif (TCAM_BUILD_FIRMWARE_UPDATE_ONLY)

  if (TCAM_BUILD_CAMERA_IP_CONF_ONLY)
    MESSAGE(STATUS " !! Only building camera-ip-conf and associated files !!")
  endif (TCAM_BUILD_CAMERA_IP_CONF_ONLY)

  if (TCAM_BUILD_UVC_EXTENSION_LOADER_ONLY)
    MESSAGE(STATUS " !! Only building tcam-uvc-extension-loader and associated files !!")
  endif (TCAM_BUILD_UVC_EXTENSION_LOADER_ONLY)

  MESSAGE(STATUS "")
  MESSAGE(STATUS "Installation prefix:                 " ${CMAKE_INSTALL_PREFIX})
  MESSAGE(STATUS "")
  MESSAGE(STATUS "Installing binaries to:              " ${TCAM_INSTALL_BIN})
  MESSAGE(STATUS "Installing libraries to:             " ${TCAM_INSTALL_LIB})
  MESSAGE(STATUS "Installing header to:                " ${TCAM_INSTALL_INCLUDE})

  if (BUILD_V4L2 OR BUILD_LIBUSB OR TCAM_ARAVIS_USB_VISION)
    MESSAGE(STATUS "Installing udev rules to:            " ${TCAM_INSTALL_UDEV})
  endif (BUILD_V4L2 OR BUILD_LIBUSB OR TCAM_ARAVIS_USB_VISION)

  if (BUILD_ARAVIS AND BUILD_TOOLS)
  MESSAGE(STATUS "Installing systemd units to:         " ${TCAM_INSTALL_SYSTEMD})
  endif (BUILD_ARAVIS AND BUILD_TOOLS)

  if (BUILD_V4L2)
    MESSAGE(STATUS "Installing uvc-extensions to:        " ${TCAM_INSTALL_UVC_EXTENSION})
  endif (BUILD_V4L2)

  if (BUILD_TOOLS)
    MESSAGE(STATUS "Installing desktop files to:         " ${TCAM_INSTALL_DESKTOP_FILES})
  endif (BUILD_TOOLS)

  MESSAGE(STATUS "Installing data files to:            " ${TCAM_INSTALL_IMAGE_DIR})

  if (BUILD_DOCUMENTATION)
    MESSAGE(STATUS "Installing documentation to          " ${TCAM_INSTALL_DOCUMENTATION})
  endif (BUILD_DOCUMENTATION)

  if (BUILD_GST_1_0)
    MESSAGE(STATUS "Installing gstreamer-1.0 to:         " ${TCAM_INSTALL_GST_1_0})
    MESSAGE(STATUS "Installing gstreamer-1.0 header to:  " ${TCAM_INSTALL_GST_1_0_HEADER})
  endif (BUILD_GST_1_0)
  MESSAGE(STATUS "Installing gobject-introspection to: " ${TCAM_INSTALL_GIR})
  MESSAGE(STATUS "Installing introspection typelib to: " ${TCAM_INSTALL_TYPELIB})
  MESSAGE(STATUS "")

endfunction()