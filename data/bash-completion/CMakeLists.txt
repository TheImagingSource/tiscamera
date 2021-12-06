
set(completion_files "tcam-ctrl" "tcam-capture")

if (BUILD_V4L2)
endif (BUILD_V4L2)

if (BUILD_ARAVIS)
  list(APPEND completion_files "tcam-gige-daemon")
endif (BUILD_ARAVIS)


install(FILES "${completion_files}"
  DESTINATION "${TCAM_INSTALL_BASH_COMPLETION}")
