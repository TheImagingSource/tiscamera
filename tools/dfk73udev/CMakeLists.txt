
include_directories("${PROJECT_SOURCE_DIR}/src")


set(cflags "-fPIC -w -Wall -Wextra -pedantic -Wundef -Wshadow")
set(lflags "-Wl,--gc-sections -ffunction-sections -fdata-sections -l -pthread -lrt")


find_package(UDev REQUIRED QUIET)
include_directories(${UDEV_INCLUDE_DIR})
add_definitions(-DHAVE_UDEV)


add_executable(dfk73udev dfk73udev.c)
target_link_libraries(dfk73udev tcam-dfk73 ${UDEV_LIBRARIES} rt)

install(TARGETS dfk73udev
  DESTINATION ${TCAM_INSTALL_BIN}
  COMPONENT tcam-dfk73)
