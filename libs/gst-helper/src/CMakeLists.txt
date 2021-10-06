
find_package(PkgConfig REQUIRED)

# If this errors, you have not installed all necessary libraries
# This generates the variables GSTREAMER_INCLUDE_DIRS and GSTREAMER_LIBRARIES
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)

add_library( project-warnings ALIAS gst-helper-libs-warning-base )

set( libraries-include-path "${CMAKE_CURRENT_SOURCE_DIR}/../include" )

add_subdirectory( gst-helper )
add_subdirectory( tcamprop1.0_base )
add_subdirectory( tcamprop1.0_gobject )
add_subdirectory( tcamprop1.0_consumer )