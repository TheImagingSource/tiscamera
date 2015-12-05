project(showcamera)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)
find_package(PkgConfig REQUIRED)

option (USE_AFU050 "Include libafu050" OFF) 
option (USE_FRAMECOUNTER "Use the frame counter" OFF) 
option (USE_ARAVIS_GIGE "Use the Aravis Library for GigE cameras" OFF) 

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(UDev REQUIRED)

pkg_check_modules(GSTREAMER gstreamer-0.10 glib-2.0 gstreamer-base-0.10 gstreamer-plugins-base-0.10 gstreamer-interfaces-0.10 gstreamer-app-0.10)


configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
   ) 




set(showcamera_SRCS cgigecamera.cpp cpropertiesdialog.cpp showcamera.cpp main.cpp usbcamera.cpp dfkafu050.cpp CameraList.cpp camerabase.cpp property.cpp usbgeneric.cpp)
qt4_automoc(${showcamera_SRCS})
add_executable(showcamera ${showcamera_SRCS})

if( USE_AFU050 )
  message( "Include AFU050 support")
  set (EXTRA_LIBS ${EXTRA_LIBS} afu050 gstapp-0.10 )
endif( USE_AFU050 )

if( USE_FRAMECOUNTER )
  message( "Use frame counter")
endif( USE_FRAMECOUNTER )

if( USE_ARAVIS_GIGE )
  message( "Use Aravis for GigE cameras")
  set (EXTRA_ARV_LIBS ${EXTRA_ARV_LIBS} aravis-0.4 )
  set (EXTRA_ARV_LIBDIR ${EXTRA_ARV_LIBDIR} /home/bvtest/source/aravis/src/aravis/src/.libs )
  set (EXTRA_ARV_INCDIR ${EXTRA_ARV_INCDIR} /home/bvtest/source/aravis/src/ )
endif( USE_ARAVIS_GIGE )


include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${GSTREAMER_INCLUDE_DIRS} ${UDEV_INCLUDE_DIR} ${EXTRA_ARV_INCDIR})
link_directories(${QT_LIBRARY_DIRS} ${QT_QTCORE_LIBRARY_DIRS} ${QT_QTGUI_LIBRARY_DIRS} ${GSTREAMER_LIBRARY_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/lib ${EXTRA_ARV_LIBDIR} )
add_definitions(${GSTREAMER_CFLAGS_OTHER})



target_link_libraries(showcamera ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${GSTREAMER_LIBRARIES} ${UDEV_LIBRARIES} ${EXTRA_LIBS} ${EXTRA_ARV_LIBS} )

install(TARGETS showcamera RUNTIME DESTINATION bin)