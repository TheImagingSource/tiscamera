
# This file contains helper functions


# helper funcion to receive the value of an arbitrary variable
function(pkg_check_variable _pkg _name)
  string(TOUPPER ${_pkg} _pkg_upper)
  string(TOUPPER ${_name} _name_upper)
  string(REPLACE "-" "_" _pkg_upper ${_pkg_upper})
  string(REPLACE "-" "_" _name_upper ${_name_upper})
  set(_output_name "${_pkg_upper}_${_name_upper}")

  find_package(PkgConfig REQUIRED)

  execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=${_name} ${_pkg}
    OUTPUT_VARIABLE _pkg_result
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  set("${_output_name}" "${_pkg_result}" CACHE STRING "pkg-config variable ${_name} of ${_pkg}")
endfunction()


# helper function to find all c/cpp files
# has to be called from project root  to find all files
function(get_all_cpp_files file_list IGNORE_PREFIX_LIST)
  file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.c *.h)
  foreach (SOURCE_FILE ${ALL_SOURCE_FILES})
    set(PREFIX_FOUND -1)
    foreach(PREFIX ${IGNORE_PREFIX_LIST})
      string(FIND ${SOURCE_FILE} ${PREFIX} PREFIX_FOUND)
      if (NOT ${PREFIX_FOUND} EQUAL -1)
        list(REMOVE_ITEM ALL_SOURCE_FILES ${SOURCE_FILE})
        break()
      endif ()

    endforeach()

  endforeach ()
  set(${file_list} ${ALL_SOURCE_FILES} PARENT_SCOPE)
endfunction()
