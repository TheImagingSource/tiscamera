

if (NOT TCAM_BUILD_UVC_EXTENSION_LOADER_ONLY)
# dfk73 special case

find_package(UDev REQUIRED QUIET)
find_package(libusb-1.0 REQUIRED QUIET)

add_library(tcam-dfk73 SHARED dfk73.c)

target_compile_definitions(tcam-dfk73
  PRIVATE -DHAVE_USB
  PRIVATE -DHAVE_UDEV)

target_include_directories(tcam-dfk73
  PRIVATE ${LIBUSB_1_INCLUDE_DIRS}
  PRIVATE ${UDEV_INCLUDE_DIR})

target_link_libraries(tcam-dfk73 ${UDEV_LIBRARIES} ${LIBUSB_1_LIBRARIES})
#set_project_warnings(tcam-dfk73)

set_property(TARGET tcam-dfk73 PROPERTY VERSION ${TCAM_VERSION})
set_property(TARGET tcam-dfk73 PROPERTY SOVERSION ${TCAM_VERSION_MAJOR})

install(TARGETS tcam-dfk73
  LIBRARY
  DESTINATION "${TCAM_INSTALL_LIB}"
  COMPONENT tcam-libdfk73)

endif (NOT TCAM_BUILD_UVC_EXTENSION_LOADER_ONLY)

# uvc-extension library

find_package(LibUuid REQUIRED QUIET)

add_library(tcam-uvc-extension STATIC
    "uvc-extension-loader.h"
    "uvc-extension-loader.cpp"
)
set_project_warnings(tcam-uvc-extension)

target_link_libraries(tcam-uvc-extension ${LIBUUID_LIBRARIES})

target_include_directories(tcam-uvc-extension
  PRIVATE ${LIBUUID_INCLUDE_DIRS}
  PRIVATE "${TCAM_BINARY_DIR}/"
  PRIVATE "${TCAM_SOURCE_DIR}/external/json")


find_package(UDev REQUIRED QUIET)


add_library( tcam-v4l2 STATIC 
    V4l2Device.cpp
    V4L2PropertyBackend.cpp
    V4L2DeviceProperties.cpp
    v4l2_property_impl.cpp
    v4l2_genicam_mapping.cpp
    v4l2_genicam_conversion.cpp
    v4l2_utils.cpp
    v4l2_api.cpp
)

target_link_libraries(tcam-v4l2
PUBLIC
  tcam-base
PRIVATE
  tcam-uvc-extension
  tcam-dfk73
  tcamprop1::base
  ${UDEV_LIBRARIES} ${LIBUSB_1_LIBRARIES}
)
  
target_compile_definitions(tcam-v4l2 PRIVATE -DHAVE_UDEV)

target_include_directories(tcam-v4l2
PRIVATE
    ${UDEV_INCLUDE_DIR}
)

set_project_warnings(tcam-v4l2)