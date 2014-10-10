

find_package(TinyXML REQUIRED)
include_directories(${TinyXML_INCLUDE_DIRS})

find_package(LibZip REQUIRED)
include_directories(${LIBZIP_INCLUDE_DIR})


# Set up QT4 and required components
find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)
include(${QT_USE_FILE})
include_directories(${QT_USE_FILE})


set(UIS
  infobox.ui
  mainwindow.ui)
qt4_wrap_ui(GENERATED_SOURCES ${UIS})


add_executable(camera-ip-conf gui_main.cpp InfoBox.cpp MainWindow.cpp UpdateHandler.cpp ${UIS} $<TARGET_OBJECTS:ip-conf-base>)


target_link_libraries(camera-ip-conf ${TinyXML_LIBRARIES})
target_link_libraries(camera-ip-conf ${LIBZIP_LIBRARY})
target_link_libraries(camera-ip-conf ${QT_LIBRARIES})


install(TARGETS camera-ip-conf DESTINATION bin)
