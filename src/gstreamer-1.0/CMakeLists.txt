
find_package(GStreamer REQUIRED QUIET)
find_package(GLIB2     REQUIRED QUIET)
find_package(GObject   REQUIRED QUIET)

include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories(${GSTREAMER_BASE_INCLUDE_DIRS})
include_directories(${GLIB2_INCLUDE_DIR})
include_directories(${GObject_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

add_library(gsttcamsrc SHARED gsttcamsrc.c)
target_link_libraries(gsttcamsrc tcam)
target_link_libraries(gsttcamsrc ${GSTREAMER_LIBRARIES})
target_link_libraries(gsttcamsrc ${GSTREAMER_BASE_LIBRARIES})
target_link_libraries(gsttcamsrc ${GLIB2_LIBRARIES})
target_link_libraries(gsttcamsrc ${GObject_LIBRARIES})
