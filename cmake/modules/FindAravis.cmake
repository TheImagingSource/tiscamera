#
# On successfull identification the following variables will be defined
#
# ARAVIS_FOUND       - system has aravis
# ARAVIS_INCLUDE_DIR - include directories
# ARAVIS_LIBRARIES   - linker flags
# ARAVIS_DEFINITIONS - Compiler flags required by aravis
#

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(aravis_PKGCONF aravis-0.4)
libfind_pkg_check_modules(aravis0_6_PKGCONF aravis-0.6)

# Include dir
find_path(aravis_INCLUDE_DIR
	NAMES
	arv.h
	PATHS
	${aravis_PKGCONF_INCLUDE_DIRS}
	${aravis0_6_PKGCONF_INCLUDE_DIRS}
	/usr/local/include
	/usr/local/include/aravis-0.4
	/usr/local/include/aravis-0.6
	/usr/include
	/usr/include/aravis-0.4
	/usr/include/aravis-0.6
)

# Finally the library itself
find_library(aravis_LIBRARIES
	NAMES
	libaravis-0.4
	libaravis-0.6
	aravis
	aravis-0.4
	aravis-0.6
	libaravis
	PATHS
	${aravis_PKGCONF_LIBRARY_DIRS}
	${aravis0_6_PKGCONF_LIBRARY_DIRS}
	/usr/local/lib
	/usr/lib
)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set TCAM_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Aravis  DEFAULT_MSG
  aravis_LIBRARIES aravis_INCLUDE_DIR)
