prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include

Name: Tcam
Description: Camera control and image acquisition library
Version: @TCAM_VERSION@
Requires: glib-2.0 gobject-2.0 gio-2.0 libxml-2.0 gthread-2.0
Libs: -L${libdir} -ltcam
Cflags: -I${includedir}/
