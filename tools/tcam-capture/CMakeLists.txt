# Copyright 2021 The Imaging Source Europe GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GStreamer REQUIRED QUIET)
find_package(GLIB2     REQUIRED QUIET)
find_package(GObject   REQUIRED QUIET)
find_package(GObjectIntrospection REQUIRED QUIET)
find_package(QT NAMES Qt5 COMPONENTS Widgets MultiMediaWidgets Multimedia REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
# find_package(Qt5MultimediaWidgets REQUIRED)

set(TCAM_CAPTURE_SOURCES
  main.cpp
  mainwindow.cpp
  mainwindow.h
  mainwindow.ui
  config.h
  definitions.h
  aboutdialog.cpp
  aboutdialog.h
  aboutdialog.ui
  optionsdialog.cpp
  optionsdialog.h
  optionsdialog.ui
  devicedialog.cpp
  devicedialog.h
  devicedialog.ui
  device.cpp
  device.h
  indexer.cpp
  indexer.h
  devicewidget.cpp
  devicewidget.h
  propertydialog.cpp
  propertydialog.h
  propertydialog.ui
  propertywidget.cpp
  propertywidget.h
  propertyworker.cpp
  propertyworker.h
  tcamslider.cpp
  tcamslider.h
  tcamspinbox.h
  tcamspinbox.cpp
  caps.cpp
  caps.h
  capswidget.cpp
  capswidget.h
  tcamcollection.cpp
  tcamcollection.h
  fpscounter.cpp
  fpscounter.h
  filename_generator.h
  filename_generator.cpp
  videosaver.h
  videosaver.cpp
  resources.qrc
  )

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
  qt_add_executable(tcam-capture
    ${TCAM_CAPTURE_SOURCES}
    )
else()
  add_executable(tcam-capture
    ${TCAM_CAPTURE_SOURCES}
    )
endif()

target_include_directories(tcam-capture PRIVATE ${GSTREAMER_INCLUDE_DIRS})
target_include_directories(tcam-capture PRIVATE ${GSTREAMER_BASE_INCLUDE_DIRS})
target_include_directories(tcam-capture PRIVATE ${GSTREAMER_VIDEO_INCLUDE_DIRS})
target_include_directories(tcam-capture PRIVATE ${GLIB2_INCLUDE_DIR})
target_include_directories(tcam-capture PRIVATE ${GObject_INCLUDE_DIR})
target_include_directories(tcam-capture PRIVATE ../../src/gstreamer-1.0/tcamgstbase)
target_include_directories(tcam-capture PRIVATE ../../src/gobject/)
target_include_directories(tcam-capture PRIVATE ../../src/)
target_include_directories(tcam-capture PRIVATE ../../external/)

target_link_libraries(tcam-capture PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(tcam-capture PRIVATE ${GOBJECT_LIBRARIES})
target_link_libraries(tcam-capture PRIVATE ${GLIB2_LIBRARIES})
target_link_libraries(tcam-capture PRIVATE ${GSTREAMER_LIBRARIES})
target_link_libraries(tcam-capture PRIVATE ${GSTREAMER_BASE_LIBRARIES})
target_link_libraries(tcam-capture PRIVATE ${GSTREAMER_VIDEO_LIBRARIES})
target_link_libraries(tcam-capture PRIVATE tcam::tcam-property)
target_link_libraries(tcam-capture PRIVATE tcamgstbase)

install(TARGETS tcam-capture
  DESTINATION ${TCAM_INSTALL_BIN}
  COMPONENT bin)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tcam-capture.desktop"
  "${CMAKE_CURRENT_BINARY_DIR}/tcam-capture.desktop" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tcam-capture.desktop"
  DESTINATION "${TCAM_INSTALL_DESKTOP_FILES}"
  COMPONENT bin)
