plugindir = @XFCE4_PANEL_PLUGINSDIR@

plugin_LTLIBRARIES =			\
	libdiskperf.la

libdiskperf_la_LDFLAGS =		\
	-avoid-version			\
	-export-dynamic			\
	-module

libdiskperf_la_SOURCES =		\
	main.c			\
	config_gui.c			\
	devperf.c			\
	debug.c

libdiskperf_la_CFLAGS =			\
	@XFCE4_PANEL_CFLAGS@

libdiskperf_la_LIBADD = 		\
	@XFCE4_PANEL_LIBS@
