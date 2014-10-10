
find_package(TinyXML REQUIRED)
include_directories(${TinyXML_INCLUDE_DIRS})

find_package(LibZip REQUIRED)
include_directories(${LIBZIP_INCLUDE_DIR})


add_library(ip-conf-base OBJECT CameraDiscovery.cpp Camera.cpp Socket.cpp NetworkInterface.cpp utils.cpp FirmwareUpgrade.cpp)

add_executable(camera-ip-conf-cli main.cpp ConsoleManager.cpp $<TARGET_OBJECTS:ip-conf-base> )

target_link_libraries(camera-ip-conf-cli ${TinyXML_LIBRARIES})
target_link_libraries(camera-ip-conf-cli ${LIBZIP_LIBRARY})

install(TARGETS camera-ip-conf-cli DESTINATION bin)

# TODO: make gui optional
add_subdirectory(gui)
