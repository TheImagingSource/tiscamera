
cmake_minimum_required(VERSION 3.10)

project( gst-helper-libs CXX C )

include( cmake/CompilerWarnings.cmake )
include( cmake/pkg-helper.cmake )

add_library( gst-helper-libs-warning-base INTERFACE )
set_project_warnings( gst-helper-libs-warning-base )

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set( gst-helper-libs-standalone true )
else()
    set( gst-helper-libs-standalone false )
endif()

if( ${gst-helper-libs-standalone} )

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


    add_compile_options( -Wno-psabi )   # disable ABI change message of g++8
    set( CMAKE_CXX_STANDARD 17 )
    set( CMAKE_CXX_STANDARD_REQUIRED ON )
    set( CMAKE_CXX_VISIBILITY_PRESET hidden )
    set( CMAKE_VISIBILITY_INLINES_HIDDEN ON )
    set( CMAKE_POSITION_INDEPENDENT_CODE ON )

    add_subdirectory( libs )
endif()

add_subdirectory( src )

if( ${gst-helper-libs-standalone} )
    add_subdirectory( test )
endif()
