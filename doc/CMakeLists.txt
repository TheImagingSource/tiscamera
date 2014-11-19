
if(BUILD_DOC)

  find_package(Doxygen REQUIRED)

  set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/doxygen.config.in)
  set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  configure_file(${doxyfile_in} ${doxyfile})

  add_custom_target(generate-doxygen
    COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)

  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)

  # add dependency to guarantee target initialization
  add_dependencies(tcam generate-doxygen)

endif()
