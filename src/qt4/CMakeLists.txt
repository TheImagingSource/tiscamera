
include_directories("${CMAKE_SOURCE_DIR}/src")

find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)
include(${QT_USE_FILE})
include_directories(${QT_USE_FILE})


set (TCAM_QT4_SRC
  propertywidget.cpp
  DeviceSelectionDialog.cpp
  # capturedisplay.cpp
  propertydialog.cpp
  VideoArea.cpp
  tcam_qt4.cpp
  )

set (TCAM_QT4_UI
  propertywidget.ui
  DeviceSelectionDialog.ui
  # capturedisplay.ui
  propertydialog.ui
  VideoArea.ui
  )

qt4_wrap_ui(TCAM_QT4_GENERATED_SOURCES ${TCAM_QT4_UI})

add_library(tcam_qt SHARED ${TCAM_QT4_SRC} ${TCAM_QT4_GENERATED_SOURCES})

target_link_libraries(tcam_qt tcam)
target_link_libraries(tcam_qt ${QT_LIBRARIES})


install(TARGETS tcam_qt DESTINATION lib)
