
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


add_subdirectory(gobject)


if (BUILD_GST_1_0)
  add_subdirectory(gstreamer-1.0)
endif (BUILD_GST_1_0)

add_subdirectory(algorithms)

# If you really want to set additional options from outside CMakeLists.txt you
# can do this:
# add_defintions($ENV{CFLAGS})

find_package(TinyXML REQUIRED QUIET)
include_directories(${TinyXML_INCLUDE_DIR})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(cflags "-fpic -w -Wall -Wextra -pedantic -Wundef -Wshadow")
set(lflags "-Wl,--gc-sections,--no-as-needed -ffunction-sections -fdata-sections -ldl")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cflags} ${lflags}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${cflags} ${lflags} -pthread")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/version.h)

set(base
  format.cpp
  logging.cpp
  DeviceInfo.cpp
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
)

if (BUILD_V4L2)

  set(lib_v4l2
    ${base}
    V4l2Device.cpp
    v4l2_utils.cpp
    v4l2library.cpp
    devicelibrary.h)

  find_package(UDev REQUIRED QUIET)
  include_directories(${UDEV_INCLUDE_DIR})
  add_definitions(-DHAVE_UDEV)

  find_package(libusb-1.0 REQUIRED QUIET)
  include_directories(${LIBUSB_1_INCLUDE_DIRS})

  add_definitions(-DHAVE_USB)

  add_library(tcam-v4l2 SHARED ${lib_v4l2})
  TARGET_LINK_LIBRARIES(tcam-v4l2 ${UDEV_LIBRARIES} ${LIBUSB_1_LIBRARIES})

  set_property(TARGET tcam-v4l2 PROPERTY VERSION ${TCAM_VERSION})
  set_property(TARGET tcam-v4l2 PROPERTY SOVERSION ${TCAM_VERSION_MAJOR})

  install(TARGETS tcam-v4l2
    LIBRARY
    DESTINATION "${TCAM_INSTALL_LIB}/tcam-${TCAM_VERSION_MAJOR}"
    COMPONENT tcam-v4l2)

  # dfk73 special case

  add_library(tcam-dfk73 SHARED dfk73.c)
  target_link_libraries(tcam-v4l2 tcam-dfk73)
  target_link_libraries(tcam-dfk73 ${UDEV_LIBRARIES} ${LIBUSB_1_LIBRARIES})

  set_property(TARGET tcam-dfk73 PROPERTY VERSION ${TCAM_VERSION})
  set_property(TARGET tcam-dfk73 PROPERTY SOVERSION ${TCAM_VERSION_MAJOR})

  install(TARGETS tcam-dfk73
    LIBRARY
    DESTINATION "${TCAM_INSTALL_LIB}"
    COMPONENT tcam-libdfk73)

endif (BUILD_V4L2)

if (BUILD_ARAVIS)

  set(lib_aravis
    ${base}
    AravisDevice.cpp
    aravis_utils.cpp
    aravislibrary.cpp
    devicelibrary.h)

  add_definitions(-DHAVE_ARAVIS)

  find_package(GObject REQUIRED QUIET) # dependency
  find_package(GLIB2 REQUIRED QUIET)   # dependency
  find_package(GIO REQUIRED QUIET)   # dependency

  if (TCAM_INTERNAL_ARAVIS)

    include_directories("${GIO_INCLUDE_DIR}")
    include_directories("${CMAKE_SOURCE_DIR}/dependencies/aravis/src")
    include_directories("${CMAKE_BINARY_DIR}/dependencies/aravis-prefix/src/aravis-build/src/")

  else (TCAM_INTERNAL_ARAVIS)

    find_package(Aravis REQUIRED QUIET)
    include_directories(${aravis_INCLUDE_DIR})

  endif(TCAM_INTERNAL_ARAVIS)

  include_directories(${GOBJECT_INCLUDE_DIR})
  include_directories(${GLIB2_INCLUDE_DIR})


  include_directories("${CMAKE_SOURCE_DIR}/tools/gige-daemon")

  add_subdirectory(tcam-network)

  add_library(tcam-aravis SHARED ${lib_aravis})

  TARGET_LINK_LIBRARIES(tcam-aravis ${GOBJECT_LIBRARIES})
  TARGET_LINK_LIBRARIES(tcam-aravis ${GLIB2_LIBRARIES})

  if (TCAM_INTERNAL_ARAVIS)
    add_dependencies(tcam-aravis aravis)
    set(ARAVIS_LIB "${CMAKE_BINARY_DIR}/dependencies/aravis-prefix/src/aravis-build/src/.libs/libaravis-0.6.a")
    #add_dependencies(tcam_aravis aravis)
    find_package(LibXml2 REQUIRED QUIET)
    find_package(PCAP REQUIRED QUIET)

    target_link_libraries(tcam-aravis "${GIO_LIBRARIES}")
    target_link_libraries(tcam-aravis "${PCAP_LIBRARIES}")

    target_link_libraries(tcam-aravis "${LIBXML2_LIBRARIES}")
    target_link_libraries(tcam-aravis "${ARAVIS_LIB}")

  else (TCAM_INTERNAL_ARAVIS)

    target_link_libraries(tcam-aravis ${aravis_LIBRARIES})

  endif (TCAM_INTERNAL_ARAVIS)


  set_property(TARGET tcam-aravis PROPERTY VERSION ${TCAM_VERSION})
  set_property(TARGET tcam-aravis PROPERTY SOVERSION ${TCAM_VERSION_MAJOR})

  install(TARGETS tcam-aravis
    LIBRARY
    DESTINATION "${TCAM_INSTALL_LIB}/tcam-${TCAM_VERSION_MAJOR}"
    COMPONENT tcam-aravis)

endif (BUILD_ARAVIS)

if (BUILD_LIBUSB)

  set(lib_libusb
    ${base}
    AFU050Device.cpp
    libusb_utils.cpp
    UsbSession.cpp
    UsbHandler.cpp
    LibusbDevice.cpp
    libusblibrary.cpp
    devicelibrary.h)

  find_package(libusb-1.0 REQUIRED QUIET)
  include_directories(${LIBUSB_1_INCLUDE_DIRS})

  add_library(tcam-libusb SHARED ${lib_libusb})
  target_link_libraries(tcam-libusb ${LIBUSB_1_LIBRARIES})

  set_property(TARGET tcam-libusb PROPERTY VERSION ${TCAM_VERSION})
  set_property(TARGET tcam-libusb PROPERTY SOVERSION ${TCAM_VERSION_MAJOR})

  install(TARGETS tcam-libusb
    LIBRARY
    DESTINATION "${TCAM_INSTALL_LIB}/tcam-${TCAM_VERSION_MAJOR}"
    COMPONENT tcam-libusb)

endif (BUILD_LIBUSB)


set(CMAKE_INSTALL_RPATH "${TCAM_INSTALL_LIB}/tcam-${TCAM_VERSION_MAJOR}")

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
  COMPONENT tcam)
