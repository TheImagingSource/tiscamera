
set(lib_v4l2
  V4l2Device.cpp
  v4l2_utils.cpp
  v4l2library.cpp)

include_directories("${CMAKE_SOURCE_DIR}/src")

find_package(UDev REQUIRED QUIET)
include_directories(${UDEV_INCLUDE_DIR})
add_definitions(-DHAVE_UDEV)

find_package(libusb-1.0 REQUIRED QUIET)
include_directories(${LIBUSB_1_INCLUDE_DIRS})

add_definitions(-DHAVE_USB)

add_library(tcam-v4l2 SHARED ${lib_v4l2} $<TARGET_OBJECTS:tcam-base>)
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
