
find_package(libusb-1.0 REQUIRED QUIET)
find_package(LibZip REQUIRED QUIET)

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

add_executable(firmware-update ${fwu-files} $<TARGET_OBJECTS:pugixml_obj>)

target_include_directories(firmware-update
  PRIVATE ${LIBUSB_1_INCLUDE_DIRS}
  PRIVATE ${LIBZIP_INCLUDE_DIR}
  PRIVATE "${TCAM_SOURCE_DIR}/external/PugiXml")

target_link_libraries(firmware-update ${LIBUSB_1_LIBRARIES} ${LIBZIP_LIBRARY} 33update)
