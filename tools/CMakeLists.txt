
if (BUILD_USB)
  ExternalProject_Add (firmware-update
    SOURCE_DIR firmware-update
    BUILD_COMMAND make
    INSTALL_DIR sbin)



  # TODO: 
endif (BUILD_USB)


if (BUILD_ARAVIS)
  # TODO:
endif (BUILD_ARAVIS)
