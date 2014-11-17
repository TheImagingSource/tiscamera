

if (BUILD_GST_0_10)
  add_subdirectory(gstreamer-0.10)
endif (BUILD_GST_0_10)

if (BUILD_GST_1_0)
  add_subdirectory(gstreamer-1.0)
endif (BUILD_GST_1_0)


# If you really want to set additional options from outside CMakeLists.txt you
# can do this:
# add_defintions($ENV{CFLAGS})

include_directories(/home/edt/include)
include_directories(/home/edt/work/drvbuildtools/trunk/lib/dutils_image/)

find_package(TinyXML REQUIRED)
include_directories(${TinyXML_INCLUDE_DIR})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set( srcs

logging.cpp
DeviceInfo.cpp
DeviceIndex.cpp
DeviceInterface.cpp
Error.cpp
FilterLoader.cpp
CaptureDevice.cpp
image_transform_base.h
MemoryBuffer.cpp
Properties.cpp
Property.cpp
PropertyGeneration.cpp
utils.cpp
VideoFormat.cpp
VideoFormatDescription.cpp
ImageSink.cpp
PipelineManager.cpp
ImageSource.cpp
serialization.cpp
filter/AutoPassFilter.cpp
filter/BayerRgbFilter.cpp
PropertyHandler.cpp
)

if (BUILD_USB)
  list(APPEND srcs V4l2Device.cpp)
  list(APPEND srcs v4l2_utils.cpp)

  find_package(UDev REQUIRED)
  include_directories(${UDEV_INCLUDE_DIR})
  add_definitions(-DHAVE_UDEV)

endif (BUILD_USB)

if (BUILD_ARAVIS)

  list(APPEND srcs AravisDevice.cpp)
  list(APPEND srcs aravis_utils.cpp)

  add_definitions(-DHAVE_ARAVIS)

  # TODO replace with find module
  find_package(PkgConfig)
  pkg_check_modules(ARAVIS REQUIRED aravis-0.4)
  include_directories(${ARAVIS_INCLUDE_DIRS})
  link_directories(${ARAVIS_LIBRARY_DIRS})

endif (BUILD_ARAVIS)

set(cflags "-fpic -w -Wall -Wextra -pedantic -Wundef -Wshadow")
set(lflags "-Wl,--gc-sections -ffunction-sections -fdata-sections")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cflags} ${lflags}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${cflags} ${lflags} -pthread")



set(PUBLIC_HEADER "tcam.h")
add_library(tcam SHARED  ${srcs} )

if (BUILD_USB)
  if (UDEV_FOUND)
    TARGET_LINK_LIBRARIES(tcam ${UDEV_LIBRARIES})
  endif (UDEV_FOUND)
endif(BUILD_USB)

TARGET_LINK_LIBRARIES(tcam ${TinyXML_LIBRARIES})

if (BUILD_ARAVIS)
  TARGET_LINK_LIBRARIES(tcam ${ARAVIS_LIBRARIES})
endif (BUILD_ARAVIS)


target_link_libraries(tcam "-L/home/edt/lib -ltis_auto_alg")


install(FILES ${PUBLIC_HEADER} DESTINATION include)


# target_compile_defiitions ()
