
find_package(libusb-1.0 REQUIRED QUIET)
include_directories(${LIBUSB_1_INCLUDE_DIRS})

find_package(LibZip REQUIRED QUIET)
include_directories(${LIBZIP_INCLUDE_DIR})

add_subdirectory(33u)

set(fwu-files
  FileHandling.cpp
  firmware-update.cpp
  UsbCamera.cpp
  Usb2Camera.cpp
  Usb3Camera.cpp
  Usb33Camera.cpp
  UsbHandler.cpp
  UsbSession.cpp
  FileHandling.cpp)

include_directories("${TCAM_SOURCE_DIR}/dependencies/PugiXml")

add_executable(firmware-update ${fwu-files} $<TARGET_OBJECTS:33update> $<TARGET_OBJECTS:pugixml_obj>)

target_link_libraries(firmware-update ${LIBUSB_1_LIBRARIES} ${LIBZIP_LIBRARY})
