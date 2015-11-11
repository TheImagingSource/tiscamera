
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

set(PLUGINS
  gsttcamsrc
  gsttcamwhitebalance
  gsttcamautoexposure
  gsttcamautofocus)

find_package(GStreamer REQUIRED QUIET)
find_package(GLIB2     REQUIRED QUIET)
find_package(GObject   REQUIRED QUIET)
find_package(GObjectIntrospection REQUIRED QUIET)


include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories(${GSTREAMER_BASE_INCLUDE_DIRS})
include_directories(${GLIB2_INCLUDE_DIR})
include_directories(${GObject_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/gobject)

set(cflags " -w -Wall -Wextra -pedantic -Wundef -Wshadow")
set(lflags "-fPIC -Wl,--gc-sections -ffunction-sections -fdata-sections")


include_directories(${INTROSPECTION_INCLUDE_DIR})

add_library(gsttcamsrc SHARED gsttcamsrc.c)

target_link_libraries(gsttcamsrc tcamprop)

add_library(gsttcamwhitebalance SHARED gsttcamwhitebalance.c image_sampling.c bayer.c)

add_library(gsttcamautoexposure SHARED gsttcamautoexposure.c image_sampling.c bayer.c)

add_library(gsttcamautofocus SHARED gsttcamautofocus.c AutoFocus.cpp auto_focus.cpp)

foreach (t IN ITEMS ${PLUGINS})

    target_link_libraries(${t} tcam)
    target_link_libraries(${t} ${GSTREAMER_LIBRARIES})
    target_link_libraries(${t} ${GSTREAMER_BASE_LIBRARIES})
    target_link_libraries(${t} ${GLIB2_LIBRARIES})
    target_link_libraries(${t} ${GObject_LIBRARIES})

endforeach (t)

install(TARGETS ${PLUGINS} DESTINATION lib/gstreamer-1.0)


# add_custom_target(gsttcam-0.1-gir tcam-gobject)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/src/gstreamer-1.0/GstTcam.gir
  DEPENDS gsttcamsrc
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/gstreamer-1.0/
  COMMAND
  ${INTROSPECTION_SCANNER}
  gsttcamsrc.c gsttcamsrc.h ${CMAKE_SOURCE_DIR}/src/gobject/tcam-gobject.h
  --namespace=GstTcam
  --nsversion=0.1
  --warn-all
  --verbose
  --include=GObject-2.0
  -I${CMAKE_SOURCE_DIR}/src
  -I${CMAKE_SOURCE_DIR}/src/gobject
  --pkg=gobject-2.0
  --pkg=gstreamer-1.0
  --pkg=gobject-introspection-1.0
  --library=tcam -L${CMAKE_BINARY_DIR}/src
  --library=gsttcamsrc -L${CMAKE_BINARY_DIR}/src/gstreamer-1.0
  --output=${CMAKE_BINARY_DIR}/src/gstreamer-1.0/GstTcam.gir
  )

#add_dependencies(create_gir tcam-gobject)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/src/gstreamer-1.0/gsttcam.typelib
  DEPENDS ${CMAKE_BINARY_DIR}/src/gstreamer-1.0/GstTcam.gir
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/src/gstreamer-1.0/
  COMMAND
  ${INTROSPECTION_COMPILER}
  --includedir=${CMAKE_BINARY_DIR}/src/gstreamer-1.0/
  ${CMAKE_BINARY_DIR}/src/gstreamer-1.0/GstTcam.gir
  -o ${CMAKE_BINARY_DIR}/src/gstreamer-1.0/gsttcam.typelib
  )

#add_dependencies(create_typelib create_gir)

add_custom_target(create_gstgobject ALL DEPENDS ${CMAKE_BINARY_DIR}/src/gstreamer-1.0/gsttcam.typelib)
