

add_library( dutils_project_options INTERFACE )
#set_target_properties( dutils_project_options PROPERTIES VISIBILITY_PRESET hidden )
target_compile_options( dutils_project_options INTERFACE -fvisibility=hidden -ffast-math )
add_library( dutils_img::project_options ALIAS dutils_project_options )

add_library( dutils_project_warnings INTERFACE )
set_project_warnings( dutils_project_warnings )
add_library( dutils_img::project_warnings ALIAS dutils_project_warnings )


add_library( dutils_img_base INTERFACE )

target_include_directories( dutils_img_base INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/" )

#target_link_libraries( dutils_img_base INTERFACE 	dutils_img::project_options )

add_library(dutils_img::img ALIAS dutils_img_base)

add_subdirectory( src/dutils_img_base )
add_subdirectory( src/dutils_img_filter )
add_subdirectory( src/dutils_img_pipe )

