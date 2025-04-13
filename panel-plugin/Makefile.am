AM_CPPFLAGS =							\
	-I$(top_srcdir)						\
	-DG_LOG_DOMAIN=\"xfce4-diskperf-plugin\"		\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"

#
# Diskperf Plugin
#
plugindir = $(libdir)/xfce4/panel/plugins

plugin_LTLIBRARIES = libdiskperf.la

libdiskperf_la_SOURCES =					\
	main.c							\
	config_gui.c						\
	config_gui.h						\
	devperf.c						\
	devperf.h

libdiskperf_la_CFLAGS =						\
	$(GLIB_CFLAGS)						\
	$(GTK_CFLAGS)						\
	$(LIBXFCE4PANEL_CFLAGS)					\
	$(LIBXFCE4UI_CFLAGS)					\
	$(LIBXFCE4UTIL_CFLAGS)

libdiskperf_la_LIBADD=						\
	$(GLIB_LIBS)						\
	$(GTK_LIBS)						\
	$(LIBM)							\
	$(LIBXFCE4PANEL_LIBS)					\
	$(LIBXFCE4UI_LIBS)					\
	$(LIBXFCE4UTIL_LIBS)

libdiskperf_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined \
	-export-symbols-regex '^xfce_panel_module_(preinit|init|construct)' \
	$(PLATFORM_LDFLAGS)

if DARWIN
libdiskperf_la_LDFLAGS += \
	-framework CoreFoundation \
	-framework IOKit
endif

#
# Desktop file
#
desktopdir = $(datadir)/xfce4/panel/plugins
desktop_in_files = diskperf.desktop.in

desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

$(desktop_DATA): $(desktop_in_files) Makefile
	$(AM_V_GEN)$(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

EXTRA_DIST = \
	$(desktop_in_files) \
	meson.build \
	$(NULL)

DISTCLEANFILES = $(desktop_DATA)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
