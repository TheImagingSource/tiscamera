
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

set(cflags "-fpic -w -Wall -Wextra -pedantic -Wundef -Wshadow")
set(lflags "-Wl,--gc-sections,--no-as-needed -ffunction-sections -fdata-sections -ldl")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cflags} ${lflags}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${cflags} ${lflags} -pthread")

set(base
  format.cpp
  logging.cpp
  DeviceInfo.cpp
  # Error.cpp
  image_transform_base.h
  MemoryBuffer.cpp
  Properties.cpp
  Property.cpp
  PropertyGeneration.cpp
  utils.cpp
  VideoFormat.cpp
  VideoFormatDescription.cpp
  ImageSink.cpp)

set( srcs
  ${base}
  BackendLoader.cpp
  DeviceIndex.cpp
  DeviceInterface.cpp
  CaptureDevice.cpp
  CaptureDeviceImpl.cpp
  PipelineManager.cpp
  ImageSource.cpp
  serialization.cpp
  PropertyHandler.cpp
  public_utils.cpp
  gsttcambase.c
)

if (BUILD_V4L2)

  set(lib_v4l2
    ${base}
    V4l2Device.cpp
    v4l2_utils.cpp
    v4l2library.cpp
    devicelibrary.h)

  find_package(UDev REQUIRED)
  include_directories(${UDEV_INCLUDE_DIR})
  add_definitions(-DHAVE_UDEV)

  add_definitions(-DHAVE_USB)

  add_library(tcam-v4l2 SHARED ${lib_v4l2})
  TARGET_LINK_LIBRARIES(tcam-v4l2 ${UDEV_LIBRARIES})

  set_property(TARGET tcam-v4l2 PROPERTY VERSION ${TCAM_VERSION})
  set_property(TARGET tcam-v4l2 PROPERTY SOVERSION ${TCAM_VERSION_MAJOR})

  install(TARGETS tcam-v4l2
    LIBRARY
    DESTINATION "${TCAM_INSTALL_LIB}"
    COMPONENT libv4l2)

endif (BUILD_V4L2)

if (BUILD_ARAVIS)

  set(lib_aravis
    ${base}
    AravisDevice.cpp
    aravis_utils.cpp
    aravislibrary.cpp
    devicelibrary.h)

  add_definitions(-DHAVE_ARAVIS)

  find_package(Aravis REQUIRED)
  find_package(GObject REQUIRED) # dependency
  find_package(GLIB2 REQUIRED)   # dependency

  include_directories(${aravis_INCLUDE_DIR})
  include_directories(${GOBJECT_INCLUDE_DIR})
  include_directories(${GLIB2_INCLUDE_DIR})


  include_directories("${CMAKE_SOURCE_DIR}/tools/gige-daemon")

  add_subdirectory(tcam-network)

  add_library(tcam-aravis SHARED ${lib_aravis})
  TARGET_LINK_LIBRARIES(tcam-aravis ${aravis_LIBRARIES})
  TARGET_LINK_LIBRARIES(tcam-aravis ${GObject_LIBRARIES})
  TARGET_LINK_LIBRARIES(tcam-aravis ${GLIB2_LIBRARIES})

  set_property(TARGET tcam-aravis PROPERTY VERSION ${TCAM_VERSION})
  set_property(TARGET tcam-aravis PROPERTY SOVERSION ${TCAM_VERSION_MAJOR})

  install(TARGETS tcam-aravis
    LIBRARY
    DESTINATION "${TCAM_INSTALL_LIB}"
    COMPONENT libaravis)

endif (BUILD_ARAVIS)


set(PUBLIC_HEADER tcam.h)
add_library(tcam SHARED ${srcs})

set_property(TARGET tcam PROPERTY VERSION ${TCAM_VERSION})
set_property(TARGET tcam PROPERTY SOVERSION ${TCAM_VERSION_MAJOR})


TARGET_LINK_LIBRARIES(tcam ${TinyXML_LIBRARIES})

install(FILES ${PUBLIC_HEADER}
  DESTINATION "${TCAM_INSTALL_INCLUDE}")

install(TARGETS tcam
  LIBRARY
  DESTINATION "${TCAM_INSTALL_LIB}"
  COMPONENT lib)
