
set(completion_files "tcam-ctrl" "tcam-capture")

if (TCAM_BUILD_ARAVIS)
  list(APPEND completion_files "tcam-gige-daemon")
endif (TCAM_BUILD_ARAVIS)


foreach (file "${completion_files}")
  install(FILES ${file}
    DESTINATION "${TCAM_INSTALL_BASH_COMPLETION}"
    COMPONENT bin)
endforeach()
