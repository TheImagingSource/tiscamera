
# Copyright 2014 The Imaging Source Europe GmbH
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

find_package(GObject   REQUIRED QUIET)
find_package(GLIB2     REQUIRED QUIET)
find_package(GObjectIntrospection REQUIRED QUIET)

add_library(tcamprop SHARED
    
    "Tcam-1.0.h"
    "tcam-property-1.0.h"
    "tcam-property-1.0.c"
)

target_include_directories( tcamprop PUBLIC ${GLIB2_INCLUDE_DIR})
target_include_directories( tcamprop PUBLIC ${GObject_INCLUDE_DIR})

target_include_directories(tcamprop PUBLIC "${CMAKE_CURRENT_DIRECTORY}")

target_link_libraries(tcamprop ${GLIB2_LIBRARIES})
target_link_libraries(tcamprop ${GObject_LIBRARIES})
target_link_libraries(tcamprop ${INTROSPECTION_LIBS})


# version number for gobject introspection API
set(TCAM_GI_API_VERSION "1.0" CACHE STRING "Version Number for Gobject Introspection API")

set_property(TARGET tcamprop PROPERTY VERSION ${TCAM_GI_API_VERSION})
set_property(TARGET tcamprop PROPERTY SOVERSION ${TCAM_GI_API_VERSION})

# projects like tiscamera
# might use a different output directory
if (CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set(tcamprop_dir ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
else ()
  set(tcamprop_dir ${CMAKE_CURRENT_BINARY_DIR})
endif (CMAKE_LIBRARY_OUTPUT_DIRECTORY)


add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/Tcam-${TCAM_GI_API_VERSION}.gir"
  DEPENDS tcamprop
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMAND
  ${INTROSPECTION_SCANNER}
  Tcam-1.0.h tcam-property-1.0.h tcam-property-1.0.c
  --namespace=Tcam
  --nsversion=${TCAM_GI_API_VERSION}
  --warn-all
  --verbose
  --include=GObject-2.0
  -I${CMAKE_CURRENT_SOURCE_DIR}
  --pkg=gobject-2.0
  --library=tcamprop -L${tcamprop_dir}
  --output="${CMAKE_CURRENT_BINARY_DIR}/Tcam-${TCAM_GI_API_VERSION}.gir"
  )

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/Tcam-${TCAM_GI_API_VERSION}.typelib"
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/Tcam-${TCAM_GI_API_VERSION}.gir"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  COMMAND
  ${INTROSPECTION_COMPILER}
  --includedir=${CMAKE_CURRENT_BINARY_DIR}/src/gobject
  "${CMAKE_CURRENT_BINARY_DIR}/Tcam-${TCAM_GI_API_VERSION}.gir"
  -o "${CMAKE_CURRENT_BINARY_DIR}/Tcam-${TCAM_GI_API_VERSION}.typelib"
  )

add_custom_target(create_gobject ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Tcam-${TCAM_GI_API_VERSION}.typelib)

install(TARGETS tcamprop LIBRARY
  DESTINATION ${TCAMPROP_INSTALL_LIB}
  COMPONENT tcam-introspection)

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
pkg_check_variable(gobject-introspection-1.0 girdir)
pkg_check_variable(gobject-introspection-1.0 typelibdir)

if (GOBJECT_INTROSPECTION_1.0_GIRDIR)
  set(TCAMPROP_INSTALL_GIR ${GOBJECT_INTROSPECTION_1.0_GIRDIR} CACHE PATH "gobject introspection installation path")
else()
  set(TCAMPROP_INSTALL_GIR "${CMAKE_INSTALL_PREFIX}/share/gir-1.0/" CACHE PATH "gobject introspection installation path")
endif()

if (GOBJECT_INTROSPECTION_1.0_TYPELIBDIR)
  set(TCAMPROP_INSTALL_TYPELIB ${GOBJECT_INTROSPECTION_1.0_TYPELIBDIR} CACHE PATH "gobject introspection typelib installation path")
else()
  set(TCAMPROP_INSTALL_TYPELIB "${CMAKE_INSTALL_PREFIX}/lib/girepository-1.0" CACHE PATH "gobject introspection typelib installation path")
endif()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Tcam-${TCAM_GI_API_VERSION}.gir
  DESTINATION ${TCAMPROP_INSTALL_GIR}
  COMPONENT tcam-introspection)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Tcam-${TCAM_GI_API_VERSION}.typelib
  DESTINATION ${TCAMPROP_INSTALL_TYPELIB}
  COMPONENT tcam-introspection)
install(FILES tcam-property-1.0.h Tcam-${TCAM_GI_API_VERSION}.h
  DESTINATION ${TCAMPROP_INSTALL_INCLUDE}
  COMPONENT tcam-introspection)
