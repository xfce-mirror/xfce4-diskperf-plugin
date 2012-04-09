INCLUDES =							\
	-I$(top_srcdir)						\
	-DG_LOG_DOMAIN=\"xfce4-diskperf-plugin\"		\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"

#
# Diskperf Plugin
#
plugindir = $(libexecdir)/xfce4/panel/plugins

plugin_LTLIBRARIES = libdiskperf.la

libdiskperf_la_SOURCES =					\
	main.c							\
	config_gui.c						\
	config_gui.h						\
	devperf.c						\
	devperf.h

libdiskperf_la_CFLAGS =						\
	$(LIBXFCE4PANEL_CFLAGS)					\
	$(LIBXFCE4UI_CFLAGS)

libdiskperf_la_LIBADD=						\
	$(LIBM)							\
	$(LIBXFCE4PANEL_LIBS)					\
	$(LIBXFCE4UI_LIBS)

libdiskperf_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined \
	-export-symbols-regex '^xfce_panel_module_(preinit|init|construct)' \
	$(PLATFORM_LDFLAGS)

#
# Desktop file
#
desktopdir = $(datadir)/xfce4/panel/plugins
desktop_in_files = diskperf.desktop.in

desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = $(desktop_in_files)

DISTCLEANFILES = $(desktop_DATA)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
