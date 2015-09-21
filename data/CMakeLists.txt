

add_subdirectory(uvc-extensions)

add_subdirectory(udev)

# define external dependencies for linkage
set(tcam_pkgconfig_dependencies "tinyxml")

if (BUILD_ARAVIS)
  set(tcam_pkgconfig_dependencies "${tcam_pkgconfig_dependencies} aravis-0.4")
endif (BUILD_ARAVIS)

if (BUILD_USB)
  set(tcam_pkgconfig_dependencies "${tcam_pkgconfig_dependencies} libudev")
endif (BUILD_USB)

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/tcam.pc.cmake  ${CMAKE_CURRENT_BINARY_DIR}/tcam.pc @ONLY)
