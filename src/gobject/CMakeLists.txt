
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

include_directories(${CMAKE_SOURCE_DIR}/src/)

find_package(GObject   REQUIRED QUIET)
find_package(GLIB2     REQUIRED QUIET)
find_package(GObjectIntrospection REQUIRED QUIET)

include_directories(${GLIB2_INCLUDE_DIR})
include_directories(${GObject_INCLUDE_DIR})
# not usable with module
# include_directories(${INTROSPECTION_INCLUDE_DIR})

set(cflags "${cflags} ${INTROSPECTION_CFLAGS}")
set(cxxflags "${cxxflags} ${INTROSPECTION_CFLAGS} -std=c++11")

add_library(tcamprop SHARED tcamprop.c)

target_link_libraries(tcamprop tcam)
target_link_libraries(tcamprop ${GLIB2_LIBRARIES})
target_link_libraries(tcamprop ${GObject_LIBRARIES})
target_link_libraries(tcamprop ${INTROSPECTION_LIBS})


# version number for gobject introspection API
set(TCAM_GI_API_VERSION "0.1" CACHE STRING "Version Number for Gobject Introspection API")

set_property(TARGET tcamprop PROPERTY VERSION ${TCAM_GI_API_VERSION})
set_property(TARGET tcamprop PROPERTY SOVERSION ${TCAM_GI_API_VERSION})

#add_custom_target(tcam-0.1-gir tcam-gobject)

add_custom_command(
  OUTPUT "${CMAKE_BINARY_DIR}/src/gobject/Tcam-${TCAM_GI_API_VERSION}.gir"
  DEPENDS tcamprop
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/src/gobject"
  COMMAND
  ${INTROSPECTION_SCANNER}
  tcamprop.c tcamprop.h
  --namespace=Tcam
  --nsversion=${TCAM_GI_API_VERSION}
  --warn-all
  --verbose
  --include=GObject-2.0
  -I${CMAKE_SOURCE_DIR}/src
  --pkg=gobject-2.0
  --library=tcamprop -L${CMAKE_BINARY_DIR}/src/gobject
  --output="${CMAKE_BINARY_DIR}/src/gobject/Tcam-${TCAM_GI_API_VERSION}.gir"
  )

#add_dependencies(create_gir tcam-gobject)

add_custom_command(
  OUTPUT "${CMAKE_BINARY_DIR}/src/gobject/Tcam-${TCAM_GI_API_VERSION}.typelib"
  DEPENDS "${CMAKE_BINARY_DIR}/src/gobject/Tcam-${TCAM_GI_API_VERSION}.gir"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/src/gobject"
  COMMAND
  ${INTROSPECTION_COMPILER}
  --includedir=${CMAKE_BINARY_DIR}/src/gobject
  "${CMAKE_BINARY_DIR}/src/gobject/Tcam-${TCAM_GI_API_VERSION}.gir"
  -o "${CMAKE_BINARY_DIR}/src/gobject/Tcam-${TCAM_GI_API_VERSION}.typelib"
  )

#add_dependencies(create_typelib create_gir)

add_custom_target(create_gobject ALL DEPENDS ${CMAKE_BINARY_DIR}/src/gobject/Tcam-${TCAM_GI_API_VERSION}.typelib)

install(TARGETS tcamprop LIBRARY
  DESTINATION ${TCAM_INSTALL_LIB}
  COMPONENT tcam-introspection)

# check for better suited install directories
pkg_check_variable(gobject-introspection-1.0 girdir)
pkg_check_variable(gobject-introspection-1.0 typelibdir)

if (GOBJECT_INTROSPECTION_1.0_GIRDIR)
  set(TCAM_INSTALL_GIR ${GOBJECT_INTROSPECTION_1.0_GIRDIR} CACHE STRING "gobject introspection installation path")
else()
  set(TCAM_INSTALL_GIR "${CMAKE_INSTALL_PREFIX}/share/gir-1.0/" CACHE STRING "gobject introspection installation path")
endif()

if (GOBJECT_INTROSPECTION_1.0_TYPELIBDIR)
  set(TCAM_INSTALL_TYPELIB ${GOBJECT_INTROSPECTION_1.0_TYPELIBDIR} CACHE STRING "gobject introspection typelib installation path")
else()
  set(TCAM_INSTALL_TYPELIB "${CMAKE_INSTALL_PREFIX}/lib/girepository-1.0" CACHE STRING "gobject introspection typelib installation path")
endif()

install(FILES ${CMAKE_BINARY_DIR}/src/gobject/Tcam-0.1.gir
  DESTINATION ${TCAM_INSTALL_GIR}
  COMPONENT tcam-introspection)
install(FILES ${CMAKE_BINARY_DIR}/src/gobject/Tcam-0.1.typelib
  DESTINATION ${TCAM_INSTALL_TYPELIB}
  COMPONENT tcam-introspection)
install(FILES tcamprop.h
  DESTINATION ${TCAM_INSTALL_INCLUDE}
  COMPONENT tcam-introspection)
