
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
  gsttcamautofocus
  gsttcambin
  )

find_package(GStreamer REQUIRED QUIET)
find_package(GLIB2     REQUIRED QUIET)
find_package(GObject   REQUIRED QUIET)
find_package(GObjectIntrospection REQUIRED QUIET)


include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories(${GSTREAMER_BASE_INCLUDE_DIRS})
include_directories(${GSTREAMER_VIDEO_INCLUDE_DIRS})
include_directories(${GLIB2_INCLUDE_DIR})
include_directories(${GObject_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/gobject)

set(cflags " -w -Wall -Wextra -pedantic -Wundef -Wshadow")
set(lflags "-fPIC -Wl,--gc-sections -ffunction-sections -fdata-sections")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cflags} ${lflags} -fpermissive")


include_directories(${INTROSPECTION_INCLUDE_DIR})

add_library(gsttcamsrc SHARED gsttcamsrc.cpp)

target_link_libraries(gsttcamsrc tcamprop)

add_library(gsttcamwhitebalance SHARED gsttcamwhitebalance.cpp image_sampling.c bayer.c)

add_library(gsttcamautoexposure SHARED gsttcamautoexposure.cpp image_sampling.c bayer.c)

add_library(gsttcamautofocus SHARED gsttcamautofocus.cpp AutoFocus.cpp auto_focus.cpp)

add_library(gsttcambin SHARED gsttcambin.cpp)

foreach (t IN ITEMS ${PLUGINS})

    target_link_libraries(${t} tcam)
    target_link_libraries(${t} ${GSTREAMER_LIBRARIES})
    target_link_libraries(${t} ${GSTREAMER_BASE_LIBRARIES})
    target_link_libraries(${t} ${GSTREAMER_VIDEO_LIBRARIES})
    target_link_libraries(${t} ${GLIB2_LIBRARIES})
    target_link_libraries(${t} ${GObject_LIBRARIES})

endforeach (t)

install(TARGETS ${PLUGINS} DESTINATION ${TCAM_INSTALL_GST_1_0})
