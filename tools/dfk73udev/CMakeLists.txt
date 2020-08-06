
include_directories("${TCAM_SOURCE_DIR}/src")
include_directories("${TCAM_SOURCE_DIR}/src/v4l2")

find_package(UDev REQUIRED QUIET)
include_directories(${UDEV_INCLUDE_DIR})
add_definitions(-DHAVE_UDEV)


add_executable(dfk73udev dfk73udev.c)
target_link_libraries(dfk73udev tcam-dfk73 ${UDEV_LIBRARIES} rt)

install(TARGETS dfk73udev
  DESTINATION ${TCAM_INSTALL_BIN}
  COMPONENT tcam-dfk73)
