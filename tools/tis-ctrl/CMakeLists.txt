
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${CMAKE_SOURCE_DIR}/src")

add_executable(tis-ctrl main.cpp properties.cpp formats.cpp multimedia.cpp)
target_link_libraries(tis-ctrl tis_imaging)
