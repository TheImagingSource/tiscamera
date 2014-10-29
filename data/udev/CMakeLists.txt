

set(RULE_FILES 80-theimagingsource-cameras.rules)

set(UDEV_INSTALL_DIR udev/rules.d)

install(FILES ${RULE_FILES} DESTINATION ${UDEV_INSTALL_DIR})
