
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

add_library(tcam-gobject SHARED tcam-gobject.cpp)

target_link_libraries(tcam-gobject tcam)
target_link_libraries(tcam-gobject ${GLIB2_LIBRARIES})
target_link_libraries(tcam-gobject ${GObject_LIBRARIES})
target_link_libraries(tcam-gobject ${INTROSPECTION_LIBS})


#add_custom_target(tcam-0.1-gir tcam-gobject)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/src/gobject/Tcam.gir
  DEPENDS tcam-gobject
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/gobject
  COMMAND
  ${INTROSPECTION_SCANNER}
  tcam-gobject.cpp tcam-gobject.h
  --namespace=Tcam
  --nsversion=0.1
  --warn-all
  --verbose
  --include=GObject-2.0
  -I${CMAKE_SOURCE_DIR}/src
  --pkg=gobject-2.0
  --library=tcam -L${CMAKE_BINARY_DIR}/src
  --library=tcam-gobject -L${CMAKE_BINARY_DIR}/src/gobject
  --output=${CMAKE_BINARY_DIR}/src/gobject/Tcam.gir
  )

#add_dependencies(create_gir tcam-gobject)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/src/gobject/tcam.typelib
  DEPENDS ${CMAKE_BINARY_DIR}/src/gobject/Tcam.gir
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/src/gobject
  COMMAND
  ${INTROSPECTION_COMPILER}
  --includedir=${CMAKE_BINARY_DIR}/src/gobject
  ${CMAKE_BINARY_DIR}/src/gobject/Tcam.gir
  -o ${CMAKE_BINARY_DIR}/src/gobject/tcam.typelib
  )

#add_dependencies(create_typelib create_gir)

#add_custom_target(create_gobject ALL DEPENDS ${CMAKE_BINARY_DIR}/src/gobject/tcam.typelib)
