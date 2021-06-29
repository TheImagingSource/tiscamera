
# This file contains helper functions


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
