
include("functions")

# get_all_cpp_files2(ALL_PROJECT_CPP_FILES)
get_all_cpp_files(ALL_PROJECT_CPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/external;${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_SOURCE_DIR}/tools/firmware-update/33u/lib;${CMAKE_CURRENT_SOURCE_DIR}/libs;")

add_custom_target(clang-format
  COMMAND clang-format -i
  ${ALL_PROJECT_CPP_FILES}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Checking source code formatting"
  )
