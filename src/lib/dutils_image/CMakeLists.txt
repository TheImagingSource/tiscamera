

add_library( dutils_img INTERFACE )

target_include_directories( dutils_img INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/" )

target_link_libraries( dutils_img
INTERFACE
#	dutils_img::project_options
)

add_library( dutils::dutils_img ALIAS dutils_img )

add_subdirectory( src/dutils_img_base )
add_subdirectory( src/dutils_img_filter )