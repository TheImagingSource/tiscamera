
set(lib_libusb
  AFU050Device.cpp
  libusb_utils.cpp
  UsbSession.cpp
  UsbHandler.cpp
  LibusbDevice.cpp
  libusblibrary.cpp
  )

include_directories("..")

find_package(libusb-1.0 REQUIRED QUIET)
include_directories(${LIBUSB_1_INCLUDE_DIRS})

add_library(tcam-libusb SHARED ${lib_libusb} $<TARGET_OBJECTS:tcam-base>)
target_link_libraries(tcam-libusb ${LIBUSB_1_LIBRARIES})
target_link_libraries(tcam-libusb tcam)

set_property(TARGET tcam-libusb PROPERTY VERSION ${TCAM_VERSION})
set_property(TARGET tcam-libusb PROPERTY SOVERSION ${TCAM_VERSION_MAJOR})

install(TARGETS tcam-libusb
  LIBRARY
  DESTINATION "${TCAM_INSTALL_LIB}/tcam-${TCAM_VERSION_MAJOR}"
  COMPONENT tcam-libusb)
