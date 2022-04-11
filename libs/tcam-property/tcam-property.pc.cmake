prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include

Name: tcam-property
Description: Camera control and image acquisition library
Version: @TCAM_PROPERTY_VERSION@
Requires: @tcam_pkgconfig_dependencies@
Libs: -L${libdir} -ltcam-property
Cflags: -I${includedir}/
