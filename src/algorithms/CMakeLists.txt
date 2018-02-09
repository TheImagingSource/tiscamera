
include_directories("${CMAKE_SOURCE_DIR}/src")

set(srcs
  AutoFocus.cpp
  auto_focus.cpp
  image_sampling.c
  bayer.c
  whitebalance.cpp
  parallel.cpp
  work_pool.cpp)

# set(PUBLIC_HEADER tcam.h tcam_c.h)
add_library(tcam-algorithms SHARED ${srcs} $<TARGET_OBJECTS:tcam-base>)

set_property(TARGET tcam-algorithms PROPERTY VERSION ${TCAM_VERSION})
set_property(TARGET tcam-algorithms PROPERTY SOVERSION ${TCAM_VERSION_MAJOR})

install(FILES ${PUBLIC_HEADER} DESTINATION "${TCAM_INSTALL_INCLUDE}")
install(TARGETS tcam-algorithms LIBRARY DESTINATION "${TCAM_INSTALL_LIB}")
