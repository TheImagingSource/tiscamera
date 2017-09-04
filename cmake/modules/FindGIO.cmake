# - Try to find the GIO libraries
# Once done this will define
#
#  GIO_FOUND - system has GIO
#  GIO_INCLUDE_DIR - the GIO include directory
#  GIO_LIBRARIES - GIO library
#
# Copyright (c) 2010 Dario Freddi <drf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(GIO_INCLUDE_DIR AND GIO_LIBRARIES)
    # Already in cache, be silent
    set(GIO_FIND_QUIETLY TRUE)
endif(GIO_INCLUDE_DIR AND GIO_LIBRARIES)

if (NOT WIN32)
    include(UsePkgConfig)
    pkgconfig(gio-2.0 _LibGIOIncDir _LibGIOLinkDir _LibGIOLinkFlags _LibGIOCflags)
endif(NOT WIN32)

MESSAGE(STATUS "gio include dir: ${_LibGIOIncDir}")

# first try without default paths to respect PKG_CONFIG_PATH

find_path(GIO_MAIN_INCLUDE_DIR glib.h
        PATH_SUFFIXES glib-2.0
        PATHS ${_LibGIOIncDir}
        NO_DEFAULT_PATH)

find_path(GIO_MAIN_INCLUDE_DIR glib.h
        PATH_SUFFIXES glib-2.0
        PATHS ${_LibGIOIncDir} )

MESSAGE(STATUS "found gio main include dir: ${GIO_MAIN_INCLUDE_DIR}")

# search the glibconfig.h include dir under the same root where the library is found
find_library(GIO_LIBRARIES
        NAMES gio-2.0
        PATHS ${_LibGIOLinkDir}
        NO_DEFAULT_PATH)

find_library(GIO_LIBRARIES
        NAMES gio-2.0
        PATHS ${_LibGIOLinkDir})


get_filename_component(GIOLibDir "${GIO_LIBRARIES}" PATH)

find_path(GIO_INTERNAL_INCLUDE_DIR glibconfig.h
        PATH_SUFFIXES glib-2.0/include
        PATHS ${_LibGIOIncDir} "${GIOLibDir}" ${CMAKE_SYSTEM_LIBRARY_PATH}
        NO_DEFAULT_PATH)

find_path(GIO_INTERNAL_INCLUDE_DIR glibconfig.h
        PATH_SUFFIXES glib-2.0/include
        PATHS ${_LibGIOIncDir} "${GIOLibDir}" ${CMAKE_SYSTEM_LIBRARY_PATH})

set(GIO_INCLUDE_DIR "${GIO_MAIN_INCLUDE_DIR}")

# not sure if this include dir is optional or required
# for now it is optional
if(GIO_INTERNAL_INCLUDE_DIR)
    set(GIO_INCLUDE_DIR ${GIO_INCLUDE_DIR} "${GIO_INTERNAL_INCLUDE_DIR}")
endif(GIO_INTERNAL_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GIO  DEFAULT_MSG  GIO_LIBRARIES GIO_MAIN_INCLUDE_DIR)

mark_as_advanced(GIO_INCLUDE_DIR GIO_LIBRARIES)

