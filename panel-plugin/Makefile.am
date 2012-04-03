INCLUDES =							\
	-I$(top_srcdir)						\
	-DG_LOG_DOMAIN=\"xfce4-diskperf-plugin\"		\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"

#
# Diskpef Plugin
#
plugindir =							\
	$(libexecdir)/xfce4/panel-plugins

plugin_PROGRAMS =						\
	xfce4-diskperf-plugin

xfce4_diskperf_plugin_SOURCES =					\
	main.c							\
	config_gui.c						\
	config_gui.h						\
	devperf.c						\
	devperf.h

xfce4_diskperf_plugin_CFLAGS =					\
	$(LIBXFCE4PANEL_CFLAGS)					\
	$(LIBXFCE4UI_CFLAGS)

xfce4_diskperf_plugin_LDADD=					\
	$(LIBM)							\
	$(LIBXFCE4PANEL_LIBS)					\
	$(LIBXFCE4UI_LIBS)

#
# Desktop file
#
desktopdir = $(datadir)/xfce4/panel-plugins
desktop_in_in_files = diskperf.desktop.in.in

desktop_in_files =						\
	$(desktop_in_in_files:.desktop.in.in=.desktop.in)

%.desktop.in: %.desktop.in.in
	sed -e "s,\@libexecdir\@,$(libexecdir),g" < $< > $@

desktop_DATA =							\
	$(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = 							\
	$(desktop_in_in_files)

DISTCLEANFILES =						\
	$(desktop_DATA)						\
	$(desktop_in_files)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
