

include_directories("${CMAKE_SOURCE_DIR}/src")

find_package(UDev REQUIRED QUIET)
include_directories(${UDEV_INCLUDE_DIR})
add_definitions(-DHAVE_UDEV)

find_package(libusb-1.0 REQUIRED QUIET)
include_directories(${LIBUSB_1_INCLUDE_DIRS})

add_definitions(-DHAVE_USB)

# dfk73 special case

add_library(tcam-dfk73 SHARED dfk73.c)

target_link_libraries(tcam-dfk73 ${UDEV_LIBRARIES} ${LIBUSB_1_LIBRARIES})

set_property(TARGET tcam-dfk73 PROPERTY VERSION ${TCAM_VERSION})
set_property(TARGET tcam-dfk73 PROPERTY SOVERSION ${TCAM_VERSION_MAJOR})

install(TARGETS tcam-dfk73
  LIBRARY
  DESTINATION "${TCAM_INSTALL_LIB}"
  COMPONENT tcam-libdfk73)

# uvc-extension library


find_package(LibUuid REQUIRED QUIET)

include_directories(${LIBUUID_INCLUDE_DIRS})

include_directories("${CMAKE_SOURCE_DIR}/dependencies/json")

add_library(tcam-uvc-extension SHARED
  uvc-extension-loader.cpp
  $<TARGET_OBJECTS:tcam-base>) # required for tcam_xioctl

target_link_libraries(tcam-uvc-extension ${LIBUUID_LIBRARIES})
target_link_libraries(tcam-uvc-extension tcam)

install(TARGETS tcam-uvc-extension
  LIBRARY
  DESTINATION "${TCAM_INSTALL_LIB}"
  COMPONENT uvc-extension)
