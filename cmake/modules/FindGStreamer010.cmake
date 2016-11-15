# - Try to find GStreamer
# Once done this will define
#
#  GSTREAMER_FOUND - system has GStreamer
#  GSTREAMER_VERSION - version as reported by gst/gstversion.h
#  GSTREAMER_INCLUDE_DIR - the GStreamer include directory
#  GSTREAMER_LIBRARIES - the libraries needed to use GStreamer
#  GSTREAMER_DEFINITIONS - Compiler switches required for using GStreamer

# Copyright (c) 2006, Tim Beaulen <tbsc...@gmail.com>
# Copyright (c) 2011, Yury G. Kudryashov <ur...@ya.ru>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls

if( NOT GStreamer010_FIND_VERSION )
  set(GStreamer010_FIND_VERSION_MAJOR "0")
  set(GStreamer010_FIND_VERSION_MINOR "10")
  set(GStreamer010_FIND_VERSION_PATCH "0")
  set(GStreamer010_FIND_VERSION "0.10.0")
endif()

find_package(PkgConfig)
if(GStreamer010_FIND_VERSION_EXACT)
  pkg_check_modules(PC_GSTREAMER010 QUIET "gstreamer-${GStreamer_FIND_VERSION_MAJOR}.${GStreamer_FIND_VERSION_MINOR}=${GStreamer_FIND_VERSION}")
else()
  pkg_check_modules(PC_GSTREAMER010 QUIET "gstreamer-${GStreamer_FIND_VERSION_MAJOR}.${GStreamer_FIND_VERSION_MINOR}>=${GStreamer_FIND_VERSION}")
endif()
set(GSTREAMER010_DEFINITIONS ${PC_GSTREAMER010_CFLAGS_OTHER})

set(GSTREAMER010_CFLAGS ${PC_GSTREAMER010_CFLAGS})

find_path(GSTREAMER010_INCLUDE_DIR gst/gst.h
  HINTS
  ${PC_GSTREAMER010_INCLUDEDIR}
  ${PC_GSTREAMER010_INCLUDE_DIRS}
  PATH_SUFFIXES
  gstreamer-${GStreamer010_FIND_VERSION_MAJOR}.${GStreamer010_FIND_VERSION_MINOR}
  )

# TODO: Store found version number in cache or not?
if(GSTREAMER010_INCLUDE_DIR)
  file(READ ${GSTREAMER010_INCLUDE_DIR}/gst/gstversion.h GST_VERSION_CONTENT)
  string(REGEX MATCH "#define GST_VERSION_MAJOR *\\([0-9]*\\)\n" _GST_VERSION_MAJOR_MATCH ${GST_VERSION_CONTENT})
  string(REGEX MATCH "#define GST_VERSION_MINOR *\\([0-9]*\\)\n" _GST_VERSION_MINOR_MATCH ${GST_VERSION_CONTENT})
  string(REGEX MATCH "#define GST_VERSION_MICRO *\\([0-9]*\\)\n" _GST_VERSION_PATCH_MATCH ${GST_VERSION_CONTENT})

  string(REGEX REPLACE "#define GST_VERSION_MAJOR *\\(([0-9]*)\\)\n" "\\1" GSTREAMER_VERSION_MAJOR ${_GST_VERSION_MAJOR_MATCH})
  string(REGEX REPLACE "#define GST_VERSION_MINOR *\\(([0-9]*)\\)\n" "\\1" GSTREAMER_VERSION_MINOR ${_GST_VERSION_MINOR_MATCH})
  string(REGEX REPLACE "#define GST_VERSION_MICRO *\\(([0-9]*)\\)\n" "\\1" GSTREAMER_VERSION_PATCH ${_GST_VERSION_PATCH_MATCH})

  set(GSTREAMER010_VERSION "${GSTREAMER010_VERSION_MAJOR}.${GSTREAMER010_VERSION_MINOR}.${GSTREAMER010_VERSION_PATCH}")
endif()

find_library(GSTREAMER010_LIBRARIES NAMES gstreamer-${GStreamer010_FIND_VERSION_MAJOR}.${GStreamer010_FIND_VERSION_MINOR}
  HINTS
  ${PC_GSTREAMER010_LIBDIR}
  ${PC_GSTREAMER010_LIBRARY_DIRS}
  )

find_library(GSTREAMER010_BASE_LIBRARY NAMES gstbase-${GStreamer010_FIND_VERSION_MAJOR}.${GStreamer010_FIND_VERSION_MINOR}
  HINTS
  ${PC_GSTREAMER010_LIBDIR}
  ${PC_GSTREAMER010_LIBRARY_DIRS}
  )

find_library(GSTREAMER010_INTERFACE_LIBRARY NAMES gstinterfaces-${GStreamer010_FIND_VERSION_MAJOR}.${GStreamer010_FIND_VERSION_MINOR}
  HINTS
  ${PC_GSTREAMER010_LIBDIR}
  ${PC_GSTREAMER010_LIBRARY_DIRS}
  )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GStreamer010
  REQUIRED_VARS GSTREAMER010_LIBRARIES GSTREAMER010_INCLUDE_DIR
  GSTREAMER010_BASE_LIBRARY GSTREAMER010_INTERFACE_LIBRARY
  VERSION_VAR GSTREAMER010_VERSION)

mark_as_advanced(GSTREAMER010_INCLUDE_DIR GSTREAMER010_LIBRARIES GSTREAMER010_BASE_LIBRARY GSTREAMER010_INTERFACE_LIBRARY)
