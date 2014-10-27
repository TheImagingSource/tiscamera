

add_subdirectory(tcam-ctrl)

if (BUILD_USB)
  add_subdirectory(firmware-update)
endif (BUILD_USB)

if (BUILD_ARAVIS)
  add_subdirectory(camera-ip-conf)
endif (BUILD_ARAVIS)
