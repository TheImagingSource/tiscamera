
find_package(GStreamer REQUIRED QUIET)
find_package(GLIB2     REQUIRED QUIET)
find_package(GObject   REQUIRED QUIET)

include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories(${GSTREAMER_BASE_INCLUDE_DIRS})
include_directories(${GLIB2_INCLUDE_DIR})
include_directories(${GObject_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

set(cflags " -w -Wall -Wextra -pedantic -Wundef -Wshadow")
set(lflags "-fPIC -Wl,--gc-sections -ffunction-sections -fdata-sections")

add_library(gsttcamsrc SHARED gsttcamsrc.c)
target_link_libraries(gsttcamsrc tcam)
target_link_libraries(gsttcamsrc ${GSTREAMER_LIBRARIES})
target_link_libraries(gsttcamsrc ${GSTREAMER_BASE_LIBRARIES})
target_link_libraries(gsttcamsrc ${GLIB2_LIBRARIES})
target_link_libraries(gsttcamsrc ${GObject_LIBRARIES})

add_library(gsttcamwhitebalance SHARED gsttcamwhitebalance.c image_sampling.c bayer.c)
target_link_libraries(gsttcamwhitebalance tcam)
target_link_libraries(gsttcamwhitebalance ${GSTREAMER_LIBRARIES})
target_link_libraries(gsttcamwhitebalance ${GSTREAMER_BASE_LIBRARIES})
target_link_libraries(gsttcamwhitebalance ${GLIB2_LIBRARIES})
target_link_libraries(gsttcamwhitebalance ${GObject_LIBRARIES})

add_library(gsttcamautoexposure SHARED gsttcamautoexposure.c image_sampling.c bayer.c)
target_link_libraries(gsttcamautoexposure tcam)
target_link_libraries(gsttcamautoexposure ${GSTREAMER_LIBRARIES})
target_link_libraries(gsttcamautoexposure ${GSTREAMER_BASE_LIBRARIES})
target_link_libraries(gsttcamautoexposure ${GLIB2_LIBRARIES})
target_link_libraries(gsttcamautoexposure ${GObject_LIBRARIES})


