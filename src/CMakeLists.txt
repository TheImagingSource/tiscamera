

if (BUILD_GST_0_10)
  add_subdirectory(gstreamer-0.10)
endif (BUILD_GST_0_10)

if (BUILD_GST_1_0)
  add_subdirectory(gstreamer-1.0)
endif (BUILD_GST_1_0)

add_subdirectory(filter)


# If you really want to set additional options from outside CMakeLists.txt you
# can do this:

# add_defintions($ENV{CFLAGS})

# dependencies

find_package(UDev REQUIRED)
include_directories(${UDEV_INCLUDE_DIR})
add_definitions(-DHAVE_UDEV)


set(CMAKE_INCLUDE_CURRENT_DIR ON)

# TODO flag settings

set( srcs

tis_logging.cpp
CaptureDevice.cpp
device_discovery.cpp
DeviceInterface.cpp
Error.cpp
FilterLoader.cpp
Grabber.cpp
image_transform_base.h
MemoryBuffer.cpp
Properties.cpp
Property.cpp
PropertyGeneration.cpp
tis_utils.cpp
V4l2Device.cpp
VideoFormat.cpp
VideoFormatDescription.cpp
ImageSink.cpp
PipelineManager.cpp
ImageSource.cpp
)


if (BUILD_ARAVIS)

  list(APPEND srcs AravisDevice.cpp)
  
  # find_package(Aravis REQUIRED)
  # include_directories(${aravis_INCLUDE_DIR})
  # # include_directories(/usr/include/aravis-0.4)

  # message( ===== ${aravis_INCLUDE_DIR} )
  # message( ===== ${aravis_LIBRARIES} === ${aravis_LIBS})
  # set( CMAKE_C_FLAGS ${ARAVIS_DEFINITIONS})
  # find_package(GLIB2 REQUIRED)
  # include_directories(${GLIB2_INCLUDE_DIR})
  add_definitions(-DHAVE_ARAVIS)

  # TODO replace with find module
  find_package(PkgConfig)
  pkg_check_modules(ARAVIS REQUIRED aravis-0.4)
  include_directories(${ARAVIS_INCLUDE_DIRS})
  link_directories(${ARAVIS_LIBRARY_DIRS})
  # target_link_libraries(map_server ${GDKPIXBUF_LIBRARIES})

  
endif (BUILD_ARAVIS)

# usb stuff 
# if ()

# end usb

# TODO:
set(cflags "-fpic -w -Wall -Wextra -pedantic -Wundef -Wshadow")
set(lflags "-Wl,--gc-sections -ffunction-sections -fdata-sections -ldl")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g3 -O0 ${cflags} ${lflags}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 ${cflags} ${lflags} -pthread")

# SET( LINK_FLAGS ${LINK_OPT})



set(PUBLIC_HEADER "tis.h")
add_library(tis_imaging SHARED  ${srcs} )

if (UDEV_FOUND)
TARGET_LINK_LIBRARIES(tis_imaging ${UDEV_LIBRARIES})
endif (UDEV_FOUND)

if (BUILD_ARAVIS)

  # TARGET_LINK_LIBRARIES(tis_imaging ${aravis_LIBRARIES})
  TARGET_LINK_LIBRARIES(tis_imaging ${ARAVIS_LIBRARIES})
endif (BUILD_ARAVIS)


install(FILES ${PUBLIC_HEADER} DESTINATION include)


# target_compile_defiitions ()
