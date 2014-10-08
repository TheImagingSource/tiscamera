
find_package(libusb-1.0 REQUIRED)
include_directories(${LIBUSB_1_INCLUDE_DIRS})

find_package(LibZip REQUIRED)
include_directories(${LIBZIP_INCLUDE_DIR})

add_executable(firmware-update FileHandling.cpp firmware-update.cpp UsbCamera.cpp Usb2Camera.cpp Usb3Camera.cpp UsbHandler.cpp UsbSession.cpp FileHandling.cpp)

target_link_libraries(firmware-update ${LIBUSB_1_LIBRARIES} ${LIBZIP_LIBRARY})

