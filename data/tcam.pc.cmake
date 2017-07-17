prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include

Name: Tcam
Description: Camera control and image acquisition library
Version: @TCAM_VERSION@
Requires: @tcam_pkgconfig_dependencies@
Libs: -L${libdir} -ltcamprop
Cflags: -I${includedir}/
