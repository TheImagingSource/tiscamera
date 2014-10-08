

add_subdirectory(tis-ctrl)

if (${BUILD_USB})
  add_subdirectory(firmware-update)
endif (${BUILD_USB})

if (${BUILD_ARAVIS})
  # TODO
endif (${BUILD_ARAVIS})
