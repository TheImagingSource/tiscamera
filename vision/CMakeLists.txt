#Specify the version being used aswell as the language
cmake_minimum_required(VERSION 2.6)

set(TISCAMERA_VERSION 1.0.0)

# Perform build configuration of dependencies

# Set up QT4 and required components
find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)
include(${QT_USE_FILE})
include_directories(${QT_USE_FILE})
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/src/qt4")


# All the project files
set(SOURCES
  main.cpp
  mainwindow.cpp)

set(HEADERS
  mainwindow.h)

set(MOCS)

set(UIS
 mainwindow.ui)


# Build and link the project

# Let QT pre-process the files and add the generated files to the source
#list
qt4_wrap_ui(GENERATED_SOURCES ${UIS})
qt4_add_resources(GENERATED_SOURCES ${RESOURCES})
qt4_wrap_cpp(GENERATED_SOURCES ${MOCS})

source_group("Generated Sources - Do Not Edit" FILES ${GENERATED_SOURCES})

# Make sure the compiler can find the pre-processed files from qt4_wrap_ui
include_directories("${CMAKE_BINARY_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/src")

# Build the app!
add_executable(tcam_vision
     MACOSX_BUNDLE
     WIN32
     # source files that are actually built direclty
     ${SOURCES}
     ${GENERATED_SOURCES}

     # items included so they show up in your IDE
     ${HEADERS}
     ${UIS}
     ${MOCS}
     ${RESOURCES})

# Link the app!
target_link_libraries(tcam_vision tcam)
target_link_libraries(tcam_vision ${QT_LIBRARIES})
target_link_libraries(tcam_vision tcam_qt)
