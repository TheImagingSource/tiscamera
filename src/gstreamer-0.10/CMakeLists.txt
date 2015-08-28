
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
  gsttiswhitebalance
  gsttisvideobufferfilter
  gsttiscolorize
  gsttis_auto_exposure
  gsttis_auto_focus)

list(REMOVE_ITEM CMAKE_C_FLAGS "-std=c99")

include_directories(${CMAKE_SOURCE_DIR}/src)


# find_package(GStreamer REQUIRED)
# find_package(GLIB2 REQUIRED)

# message("=============================== ${GSTREAMER_CFLAGS}")

# include_directories(${GSTREAMER_CFLAGS})
# include_directories(${GLIB2_INCLUDE_DIR})

find_package(GStreamer010)

find_package(PkgConfig)
pkg_check_modules(GST_0_10 REQUIRED gstreamer-0.10)
pkg_check_modules(GSTB_0_10 REQUIRED gstreamer-base-0.10)

include_directories(${GST_0_10_INCLUDE_DIRS})
include_directories(${GSTB_0_10_INCLUDE_DIRS})
include_directories(${GSTREAMER010_INCLUDE_DIR})

link_directories(${GST_0_10_LIBRARY_DIRS})
link_directories(${GSTREAMER010_LIBRARY_DIRS})
link_directories(${GSTB_0_10_LIBRARY_DIRS})


add_library(gsttiswhitebalance SHARED gsttiswhitebalance.c image_sampling.c bayer.c)

add_library(gsttisvideobufferfilter SHARED gsttisvideobufferfilter.c)

add_library(gsttiscolorize SHARED gsttiscolorize.c)
target_link_libraries(gsttiscolorize ${UDEV_LIBRARIES})

add_library(gsttis_auto_exposure SHARED gsttis_auto_exposure.c image_sampling.c bayer.c)

add_library(gsttis_auto_focus SHARED gsttis_autofocus.c AutoFocus.cpp auto_focus.cpp)


foreach (t IN ITEMS ${PLUGINS})

  target_link_libraries(${t} ${GST_0_10_LIBARIES})
  target_link_libraries(${t} ${GSTREAMER010_LIBARIES})
  target_link_libraries(${t} ${GSTB_0_10_LIBARIES})

endforeach (t)

# PLUGIN_INSTALL_DIR=$(HOME)/.gstreamer-0.10/plugins

install(TARGETS ${PLUGINS} DESTINATION lib/gstreamer-0.10)
