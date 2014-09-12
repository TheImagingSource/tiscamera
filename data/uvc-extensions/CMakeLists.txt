

# assure executable exists
# find_executable(UVCDYNCTRL uvcdynctrl)

set(EXTENSION_UNITS tisEUVC.xml usb3.xml)

install(FILES ${EXTENSION_UNITS} DESTINATION share/uvcdynctrl/data/199e/)
