dnl
dnl Copyright (c) 2004-2024 The Xfce development team
dnl

XDT_VERSION_INIT([2.7.0], [git])

dnl Initialize autoconf
AC_COPYRIGHT([Copyright (c) 2003, 2004 Roger Seguin <roger_seguin@msn.com>
              Copyright (c) 2005-2024 The Xfce development team])
AC_INIT([xfce4-diskperf-plugin], [xdt_version], [https://gitlab.xfce.org/panel-plugins/xfce4-diskperf-plugin/], [xfce4-diskperf-plugin])
AC_REVISION([xdt_version_build])
AC_PREREQ([2.69])
AC_CONFIG_MACRO_DIRS([m4])

dnl Initialize automake
AM_INIT_AUTOMAKE([1.11 dist-bzip2 tar-ustar no-dist-gzip foreign])
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE()
AM_SILENT_RULES([yes])

dnl Check for basic programs
AC_PROG_CC()
LT_PATH_LD([])
AC_PROG_INSTALL
LT_PREREQ([2.2.6])
LT_INIT([disable-static])

dnl Check for standard headers
AC_CHECK_HEADERS([sys/sysmacros.h])
LT_LIB_M
AC_SUBST(LIBM)

dnl Check for Darwin to add IOKit and CoreFoundation
AM_CONDITIONAL([DARWIN], [test x"$(uname)" = x"Darwin"])

dnl translations
GETTEXT_PACKAGE="$PACKAGE"
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Name of default gettext domain])
AC_SUBST([GETTEXT_PACKAGE])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.8])

dnl Check for required packages
XDT_CHECK_PACKAGE([GLIB], [glib-2.0], [2.50.0])
XDT_CHECK_PACKAGE([GTK], [gtk+-3.0], [3.22.0])
XDT_CHECK_PACKAGE([LIBXFCE4PANEL], [libxfce4panel-2.0], [4.16.0])
XDT_CHECK_PACKAGE([LIBXFCE4UI], [libxfce4ui-2], [4.16.0])
XDT_CHECK_PACKAGE([LIBXFCE4UTIL], [libxfce4util-1.0], [4.16.0])

dnl Check for debugging support
XDT_FEATURE_DEBUG([xdt_debug_default])

AC_CONFIG_FILES([
Makefile
panel-plugin/Makefile
po/Makefile.in
])
AC_OUTPUT
