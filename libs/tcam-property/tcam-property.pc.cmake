prefix=@CMAKE_INSTALL_PREFIX@
libdir=${prefix}/lib
includedir=${prefix}/include

Name: tcam-property
Description: Camera control and image acquisition library
Version: @TCAM_PROPERTY_VERSION@
Requires: @tcam_pkgconfig_dependencies@
Libs: -L${libdir} -ltcam-property
Cflags: -I${includedir}/
