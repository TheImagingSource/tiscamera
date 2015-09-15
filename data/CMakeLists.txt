

add_subdirectory(uvc-extensions)

add_subdirectory(udev)

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/tcam.pc.cmake  ${CMAKE_CURRENT_SOURCE_DIR}/tcam.pc @ONLY)
