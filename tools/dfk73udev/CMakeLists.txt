

find_package(UDev REQUIRED QUIET)

add_executable(dfk73udev dfk73udev.c)

target_compile_definitions(dfk73udev PRIVATE -DHAVE_UDEV)

target_include_directories(dfk73udev
  PRIVATE "${UDEV_INCLUDE_DIR}"
  PRIVATE "${TCAM_SOURCE_DIR}/src"
  PRIVATE "${TCAM_SOURCE_DIR}/src/v4l2")

target_link_libraries(dfk73udev tcam-dfk73 ${UDEV_LIBRARIES} rt)

install(TARGETS dfk73udev
  DESTINATION ${TCAM_INSTALL_BIN}
  COMPONENT tcam-dfk73)
