
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

add_subdirectory(gobject)


if (BUILD_GST_1_0)
  add_subdirectory(gstreamer-1.0)
endif (BUILD_GST_1_0)



# If you really want to set additional options from outside CMakeLists.txt you
# can do this:
# add_defintions($ENV{CFLAGS})

find_package(TinyXML REQUIRED)
include_directories(${TinyXML_INCLUDE_DIR})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set( srcs
format.cpp
logging.cpp
DeviceInfo.cpp
DeviceIndex.cpp
DeviceInterface.cpp
Error.cpp
CaptureDevice.cpp
CaptureDeviceImpl.cpp
image_transform_base.h
MemoryBuffer.cpp
Properties.cpp
Property.cpp
PropertyGeneration.cpp
utils.cpp
VideoFormat.cpp
VideoFormatDescription.cpp
ImageSink.cpp
PipelineManager.cpp
ImageSource.cpp
serialization.cpp
PropertyHandler.cpp
public_utils.cpp
gsttcambase.c
)

if (BUILD_V4L2)
  list(APPEND srcs V4l2Device.cpp)
  list(APPEND srcs v4l2_utils.cpp)

  find_package(UDev REQUIRED)
  include_directories(${UDEV_INCLUDE_DIR})
  add_definitions(-DHAVE_UDEV)

  add_definitions(-DHAVE_USB)

endif (BUILD_V4L2)

if (BUILD_ARAVIS)

  list(APPEND srcs AravisDevice.cpp)
  list(APPEND srcs aravis_utils.cpp)

  add_definitions(-DHAVE_ARAVIS)

  find_package(Aravis REQUIRED)
  find_package(GObject REQUIRED) # dependency
  find_package(GLIB2 REQUIRED)   # dependency

  include_directories(${aravis_INCLUDE_DIR})
  include_directories(${GObject_INCLUDE_DIR})
  include_directories(${GLIB2_INCLUDE_DIR})

  add_subdirectory(tcam-network)

endif (BUILD_ARAVIS)

set(cflags "-fpic -w -Wall -Wextra -pedantic -Wundef -Wshadow")
set(lflags "-Wl,--gc-sections -ffunction-sections -fdata-sections")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cflags} ${lflags}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${cflags} ${lflags} -pthread")



set(PUBLIC_HEADER tcam.h)
add_library(tcam SHARED ${srcs})

set_property(TARGET tcam PROPERTY VERSION ${TCAM_VERSION})
set_property(TARGET tcam PROPERTY SOVERSION ${TCAM_VERSION_MAJOR})


if (BUILD_V4L2)
  if (UDEV_FOUND)
    TARGET_LINK_LIBRARIES(tcam ${UDEV_LIBRARIES})
  endif (UDEV_FOUND)
endif(BUILD_V4L2)

TARGET_LINK_LIBRARIES(tcam ${TinyXML_LIBRARIES})

if (BUILD_ARAVIS)
  TARGET_LINK_LIBRARIES(tcam ${aravis_LIBRARIES})
  TARGET_LINK_LIBRARIES(tcam ${GObject_LIBRARIES})
  TARGET_LINK_LIBRARIES(tcam ${GLIB2_LIBRARIES})
endif (BUILD_ARAVIS)


install(FILES ${PUBLIC_HEADER} DESTINATION "${TCAM_INSTALL_INCLUDE}")
install(TARGETS tcam LIBRARY DESTINATION "${TCAM_INSTALL_LIB}")
