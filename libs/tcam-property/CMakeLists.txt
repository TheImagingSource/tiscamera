
cmake_minimum_required(VERSION 3.2)

project(tcam-property)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set( tcamproperty-standalone true )
else()
  set( tcamproperty-standalone false )
endif()

if( ${tcamproperty-standalone} )
  set(default_build_type "Release")

  if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)

    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
      "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
  endif()

  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr/" CACHE PATH "Common prefix for all installed files." FORCE)
  endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

endif()

if (NOT TCAM_PROPERTY_INSTALL_LIB)
  set(TCAM_PROPERTY_INSTALL_LIB "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "library installation path")
endif()

if (NOT TCAM_PROPERTY_INSTALL_INCLUDE)
  set(TCAM_PROPERTY_INSTALL_INCLUDE "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "header installation path")
endif()

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

# check for better suited install directories

if (NOT TCAM_PROPERTY_INSTALL_GIR)

  pkg_check_variable(gobject-introspection-1.0 girdir)
  if (GOBJECT_INTROSPECTION_1.0_GIRDIR)
    set(TCAM_PROPERTY_INSTALL_GIR ${GOBJECT_INTROSPECTION_1.0_GIRDIR} CACHE PATH "gobject introspection installation path")
  else()
    set(TCAM_PROPERTY_INSTALL_GIR "${CMAKE_INSTALL_PREFIX}/share/gir-1.0/" CACHE PATH "gobject introspection installation path")
  endif()
endif()


if (NOT TCAM_PROPERTY_INSTALL_TYPELIB)

  pkg_check_variable(gobject-introspection-1.0 typelibdir)
  if (GOBJECT_INTROSPECTION_1.0_TYPELIBDIR)
    set(TCAM_PROPERTY_INSTALL_TYPELIB ${GOBJECT_INTROSPECTION_1.0_TYPELIBDIR} CACHE PATH "gobject introspection typelib installation path")
  else()
    set(TCAM_PROPERTY_INSTALL_TYPELIB "${CMAKE_INSTALL_PREFIX}/lib/girepository-1.0" CACHE PATH "gobject introspection typelib installation path")
  endif()
endif()


if (NOT TCAM_PROPERTY_INSTALL_GST_1_0_HEADER)

  pkg_check_variable(gstreamer-1.0 pluginsdir)
  pkg_check_variable(gstreamer-1.0 includedir)

  if (GSTREAMER_1.0_PLUGINSDIR AND NOT TCAM_INSTALL_FORCE_PREFIX)
    set(TCAM_PROPERTY_INSTALL_GST_1_0 "${GSTREAMER_1.0_PLUGINSDIR}" CACHE PATH "gstreamer-1.0 module installation path")
  else()
    set(TCAM_PROPERTY_INSTALL_GST_1_0 "${CMAKE_INSTALL_PREFIX}/lib/gstreamer-1.0" CACHE PATH "gstreamer-1.0 module installation path")
  endif()

  if (GSTREAMER_1.0_INCLUDEDIR AND NOT TCAM_INSTALL_FORCE_PREFIX)
    set(TCAM_PROPERTY_INSTALL_GST_1_0_HEADER "${GSTREAMER_1.0_INCLUDEDIR}" CACHE PATH "gstreamer-1.0 header installation path")
  else()
    set(TCAM_PROPERTY_INSTALL_GST_1_0_HEADER "${CMAKE_INSTALL_PREFIX}/include/gstreamer-1.0" CACHE PATH "gstreamer-1.0 header installation path")
  endif()


  pkg_check_variable(gobject-introspection-1.0 typelibdir)
  if (GOBJECT_INTROSPECTION_1.0_TYPELIBDIR)
    set(TCAM_PROPERTY_INSTALL_GST_1_0_HEADER ${GOBJECT_INTROSPECTION_1.0_TYPELIBDIR} CACHE PATH "gobject introspection typelib installation path")
  else()
    set(TCAM_PROPERTY_INSTALL_GST_1_0_HEADER "${CMAKE_INSTALL_PREFIX}/lib/girepository-1.0" CACHE PATH "gobject introspection typelib installation path")
  endif()

endif (NOT TCAM_PROPERTY_INSTALL_GST_1_0_HEADER)


set(TCAM_PROPERTY_VERSION_MAJOR 1)
set(TCAM_PROPERTY_VERSION_MINOR 0)
set(TCAM_PROPERTY_VERSION_PATCH 0)

set(TCAM_PROPERTY_VERSION "${TCAM_PROPERTY_VERSION_MAJOR}.${TCAM_PROPERTY_VERSION_MINOR}.${TCAM_PROPERTY_VERSION_PATCH}" CACHE STRING "Version number")


add_subdirectory(src)

add_library( tcam::tcam-property ALIAS tcam-property )
add_library( tcam::tcamgststatistics ALIAS tcamgststatistics )

# uninstall target
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

# create target uninstall to deinstall tiscamera
# if tiscamera is a subproject/module
# create uninstall-tcamprop instead
if( ${tcamproperty-standalone} )
  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
else()
  add_custom_target(uninstall-tcamproperty
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
set(tcam_pkgconfig_dependencies "gobject-introspection-1.0")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tcam-property.pc.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/tcam-property-1.0.pc" @ONLY)

if (NOT TCAM_PROPERTY_INSTALL_PKG_CONFIG)

  find_program(EXECUTABLE_DPKG_ARCH dpkg-architecture
    DOC "dpkg-architecture program of Debian-based systems")

  if (EXECUTABLE_DPKG_ARCH)
    execute_process(COMMAND ${EXECUTABLE_DPKG_ARCH} -qDEB_HOST_MULTIARCH
      OUTPUT_VARIABLE DEB_HOST_MULTIARCH
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    string(REGEX REPLACE "\n$" "" DEB_HOST_MULTIARCH "${DEB_HOST_MULTIARCH}")

  endif (EXECUTABLE_DPKG_ARCH)

  if (DEB_HOST_MULTIARCH)
    set(TCAM_PROPERTY_INSTALL_PKGCONFIG "${CMAKE_INSTALL_PREFIX}/lib/${DEB_HOST_MULTIARCH}/pkgconfig"
      CACHE PATH "pkgconfig installation path")
  else (DEB_HOST_MULTIARCH)
    set(TCAM_PROPERTY_INSTALL_PKGCONFIG "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig"
      CACHE PATH "pkgconfig installation path")
  endif (DEB_HOST_MULTIARCH)

endif (NOT TCAM_PROPERTY_INSTALL_PKG_CONFIG)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tcam-property-1.0.pc" DESTINATION "${TCAM_PROPERTY_INSTALL_PKGCONFIG}" COMPONENT dev)

if( ${tcamproperty-standalone} )

    include(CPackComponent)

    set(CPACK_COMPONENTS_ALL tcamprop)

    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "The Imaging Source Europe GmbH <support@theimagingsource.com>")

    set(CPACK_PACKAGE_DESCRIPTION "The Imaging Source tcamprop gobject introspection library")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Library used for gobject introspection.")
    set(CPACK_PACKAGE_VENDOR "The Imaging Source Europe GmbH")
    set(CPACK_PACKAGE_CONTACT "support@theimagingsource.com")
    set(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS ON)

    set(CPACK_PACKAGE_CHECKSUM "SHA512")

    # based on current reference system
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libglib2.0-0 (>= 2.48.2), libgirepository-1.0-1 (>= 1.46.0)")

    set(CPACK_SET_DESTDIR "on")
    set(CPACK_GENERATOR "DEB")

    set(CPACK_PACKAGE_VERSION_MAJOR "${TCAM_PROPERTY_VERSION_MAJOR}")
    set(CPACK_PACKAGE_VERSION_MINOR "${TCAM_PROPERTY_VERSION_MINOR}")
    set(CPACK_PACKAGE_VERSION_PATCH "${TCAM_PROPERTY_VERSION_PATCH}")

    include( cmake/git-helper.cmake )

    git_commit_count(GIT_COMMIT_COUNT)
    # ensure package version always increases
    set(CPACK_DEBIAN_PACKAGE_VERSION "${TCAM_PROPERTY_VERSION}.${GIT_COMMIT_COUNT}")

    set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/deb/postinst.in" "${CMAKE_CURRENT_BINARY_DIR}/deb/postinst")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/deb/prerm.in" "${CMAKE_CURRENT_BINARY_DIR}/deb/prerm")

    set(CPACK_DEBIAN_PACKAGE_PROVIDES "tiscamera-tcamprop")

    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/deb/postinst;${CMAKE_CURRENT_BINARY_DIR}/deb/prerm;")


    set(CPACK_PACKAGE_NAME "tiscamera-tcamproperty")

    include(cmake/package-name.cmake)

    create_package_name(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}" "${TCAM_PROPERTY_VERSION}")

    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

    # always last
    include(CPack)

endif()
