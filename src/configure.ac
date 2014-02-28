dnl
dnl Autoconf script for The Imaging Source Gstreamer plugins
dnl
dnl Copyright 2013 The Imaging Source Europe GmbH
dnl
dnl Licensed under the Apache License, Version 2.0 (the "License");
dnl you may not use this file except in compliance with the License.
dnl You may obtain a copy of the License at
dnl
dnl http://www.apache.org/licenses/LICENSE-2.0
dnl
dnl Unless required by applicable law or agreed to in writing, software
dnl distributed under the License is distributed on an "AS IS" BASIS,
dnl WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
dnl See the License for the specific language governing permissions and
dnl limitations under the License.
dnl

AC_PREREQ([2.68])
AC_INIT([tis-gst-plugins], [0.1])

AC_CONFIG_SRCDIR([gsttiswhitebalance.c])
AC_CONFIG_HEADERS([config.h])

dnl AC_LANG(C)
AC_LANG_C


# Checks for programs.
AC_PROG_CC
AC_PROG_LN_S

dnl AM_INIT_AUTOMAKE([-Wall foreign])

ACLOCAL_FLAGS="-I /usr/share/aclocal"
PKG_CONFIG_PATH="-I /usr/share/aclocal"
PKG_CONFIG_LIBDIR="-I /usr/share/aclocal"
dnl m4_include([pkg.m4])

PKG_CHECK_MODULES([GLIB], [glib-2.0],break,AC_MSG_ERROR(Unable to locate glib module))
PKG_CHECK_MODULES([GST_BASE], [gstreamer-base-0.10],break,AC_MSG_ERROR(Unable to locate gst-base module))
PKG_CHECK_MODULES([GST], [gstreamer-0.10],break,AC_MSG_ERROR(Unable to locate gstreamer-0.10 module))

PKG_CHECK_MODULES([UDEV], [libudev], break, AC_MSG_ERROR(Unable to locate libudev module))

CFLAGS+=" $GLIB_CFLAGS $GST_BASE_CFLAGS $GST_CFLAGS $UDEV_CFLAGS -DHAVE_CONFIG_H "

# assure all checks have all necessary flags
dnl CPPFLAGS+=" $CFLAGS -DHAVE_CONFIG_H "
LDFLAGS+=" -L/home/edt/work/aravis/src/.libs $GLIB_LIBS $GST_BASE_LIBS $GST_LIBS $UDEV_LIBS"


# libusb

AC_ARG_ENABLE(aravis,
              [  --enable-aravis    build with aravis support],
              [enable_aravis=$enableval],
              [enable_aravis=no])

# Checks for header files.
AC_CHECK_HEADERS([sys/ioctl.h])

HAVE_ARV=no
LIBARV=

if test "$enable_aravis" = "yes"; then

   PKG_CHECK_MODULES([ARV], [aravis], break, AC_MSG_NOTICE(Unable to locate pkg-config for aravis.))

   AC_CHECK_LIB([aravis-0.4],
                arv_camera_set_gain,
                break,
                AC_MSG_ERROR(Unable to locate aravis libraries))

   AC_CHECK_HEADER([arv.h],
                   [HAVE_ARV=yes],
                   [HAVE_ARV=no; AC_MSG_ERROR(Unable to locate aravis header.)])

   # no error until here; aravis is working
   LDFLAGS+=" -laravis-0.4"
   AC_DEFINE([ENABLE_ARAVIS], [1])
fi

AC_SUBST([LIBS])
AC_SUBST([AM_CXXFLAGS])
AC_SUBST([AM_LDFLAGS])


AC_CONFIG_FILES([Makefile])
AC_OUTPUT

echo ""
echo "Configuration:"
echo "  Building with aravis as a dependency:  $enable_aravis"
echo ""
echo "$CFLAGS"
echo "$CPPFLAGS"
