/*  Copyright (c) 2003-2004 Roger Seguin <roger_seguin@msn.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config_gui.h"
#include "devperf.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/xfce-panel-plugin.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef LIBXFCE4PANEL_CHECK_VERSION
#if LIBXFCE4PANEL_CHECK_VERSION (4,9,0)
#define HAS_PANEL_49
#endif
#endif

#define PLUGIN_NAME	"DiskPerf"
#define BORDER          8


 /* Some platforms do not provide busy times as separate read and write
    data, but only a single value combining both */
#if  defined(__NetBSD__)
#define	SEPARATE_BUSY_TIMES	0
#elif  defined(__sun__)
#define	SEPARATE_BUSY_TIMES	0
#elif defined(__linux__)
#define	SEPARATE_BUSY_TIMES	1
#else
#define	SEPARATE_BUSY_TIMES	1
#endif


typedef GtkWidget *Widget_t;

typedef enum statistics_t {
    IO_TRANSFER,		/* MB transferred per second */
    BUSY_TIME			/* Percentage of time the device has been
				   busy */
} statistics_t;

enum {
    /* Monitor bar data */
    R_DATA,
    W_DATA,
    RW_DATA,
    NMONITORS
};

typedef enum monitor_bar_order_t {
    RW_ORDER,
    WR_ORDER
} monitor_bar_order_t;

typedef struct param_t {
    /* Configurable parameters */
    char            acDevice[64];
#if  !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__sun__)
    dev_t           st_rdev;
#endif
    int             fTitleDisplayed;
    char            acTitle[16];
    enum statistics_t
                    eStatistics;
    enum monitor_bar_order_t
                    eMonitorBarOrder;
    int             iMaxXferMBperSec;
    int             fRW_DataCombined;
    uint32_t        iPeriod_ms;
    GdkRGBA         aoColor[NMONITORS];
} param_t;

typedef struct color_selector_t {
    Widget_t        wPB;
    Widget_t        wDA;
} color_selector_t;

typedef struct conf_t {
    Widget_t        wTopLevel;
    struct gui_t    oGUI;	/* Configuration/option dialog */
    struct color_selector_t
                    aoColorWidgets[NMONITORS];	/* Color selection dialog */
    struct param_t  oParam;
} conf_t;

typedef struct perfbar_t {
    /* Individual monitor bar */
    Widget_t       *pwBar;	/* Link to monitor_t:awProgressBar */
} perfbar_t;

typedef struct monitor_t {
    /* Plugin monitor bars */
    Widget_t        wEventBox;
    Widget_t        wBox;
    Widget_t        wTitle;
    Widget_t        awProgressBar[2];	/* Physical (widget) bars */
    struct perfbar_t
                    aoPerfBar[NMONITORS];	/* Virtual bars */
    struct devperf_t
                    oPrevPerf;
} monitor_t;

typedef struct diskperf_t {
    XfcePanelPlugin *plugin;
    unsigned int    iTimerId;	/* Cyclic update */
    struct conf_t   oConf;
    struct monitor_t
                    oMonitor;
} diskperf_t;

	/**************************************************************/

static int timerNeedsUpdate = 0;

static void UpdateProgressBars(struct diskperf_t *p_poPlugin, double rw, double r, double w) {
 /* Update combined or separate progress bars with actual data */
    struct monitor_t *poMonitor = &(p_poPlugin->oMonitor);
    struct param_t *poConf = &(p_poPlugin->oConf.oParam);
    struct perfbar_t *poPerf = poMonitor->aoPerfBar;

    if (poConf->fRW_DataCombined)
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR
				       (*(poPerf[RW_DATA].pwBar)),
				       rw);
    else {
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR
				       (*(poPerf[R_DATA].pwBar)),
				       r);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR
				       (*(poPerf[W_DATA].pwBar)),
				       w);
    }
}


static int DisplayPerf (struct diskperf_t *p_poPlugin)
 /* Get the last disk perfomance data, compute the statistics and update
    the panel-docked monitor bars */
{
    struct devperf_t oPerf;
    struct param_t *poConf = &(p_poPlugin->oConf.oParam);
    struct monitor_t *poMonitor = &(p_poPlugin->oMonitor);
    struct perfbar_t *poPerf = poMonitor->aoPerfBar;
#if !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__sun__)
    struct stat     oStat;
#endif
    uint64_t        iInterval_ns, rbytes, wbytes, iRBusy_ns, iWBusy_ns;
    const double    K = 1.0 * 1000 * 1000 * 1000 / 1024 / 1024;
    /* bytes/ns --> MB/s */
    double          arPerf[NMONITORS], arBusy[NMONITORS], *prData, *pr;
    char            acToolTips[256];
    int             status, i;

    rbytes = wbytes = iRBusy_ns = iWBusy_ns = -1;
    memset (&oPerf, 0, sizeof (oPerf));
    oPerf.qlen = -1;
#if defined(__FreeBSD__) || defined (__NetBSD__) || defined(__OpenBSD__) || defined(__sun__)
    status = DevGetPerfData (poConf->acDevice, &oPerf);
#else
    if (poConf->st_rdev == 0)
    poConf->st_rdev = (stat (poConf->acDevice, &oStat) == -1 ? 0 : oStat.st_rdev);
    status = DevGetPerfData (&(poConf->st_rdev), &oPerf);
#endif
    if (status == -1) {
    snprintf (acToolTips, sizeof(acToolTips), _("%s: Device statistics unavailable."),
              poConf->acTitle);
    UpdateProgressBars(p_poPlugin, 0, 0, 0);
    gtk_widget_set_tooltip_text(GTK_WIDGET(poMonitor->wEventBox), acToolTips);

	return (-1);
    }
    if (poMonitor->oPrevPerf.timestamp_ns) {
	iInterval_ns =
	    oPerf.timestamp_ns - poMonitor->oPrevPerf.timestamp_ns;
	rbytes = oPerf.rbytes - poMonitor->oPrevPerf.rbytes;
	wbytes = oPerf.wbytes - poMonitor->oPrevPerf.wbytes;
	iRBusy_ns = oPerf.rbusy_ns - poMonitor->oPrevPerf.rbusy_ns;
	iWBusy_ns = oPerf.wbusy_ns - poMonitor->oPrevPerf.wbusy_ns;
    }
    else
	iInterval_ns = 0;
    poMonitor->oPrevPerf = oPerf;
    if (!iInterval_ns)
	return (1);

    arPerf[R_DATA] = K * rbytes / iInterval_ns;
    arPerf[W_DATA] = K * wbytes / iInterval_ns;
    arPerf[RW_DATA] = K * (rbytes + wbytes) / iInterval_ns;

    if (oPerf.qlen < 0)
	for (i = 0; i < NMONITORS; i++)
	    arBusy[i] = 0;
    else {
	arBusy[R_DATA] = (double) 100.0 *iRBusy_ns / iInterval_ns;
	arBusy[W_DATA] = (double) 100.0 *iWBusy_ns / iInterval_ns;
	arBusy[RW_DATA] =
	    (double) 100.0 *(iRBusy_ns + iWBusy_ns) / iInterval_ns;
	for (i = 0; i < NMONITORS; i++) {
	    pr = arBusy + i;
	    if (*pr > 100)
		*pr = 100;
	}
    }

    snprintf (acToolTips, sizeof(acToolTips), _("%s\n"
	     "----------------\n"
	     "I/O    (MiB/s)\n"
	     "  Read :%3.2f\n"
	     "  Write :%3.2f\n"
	     "  Total :%3.2f\n"
	     "Busy time (%c)\n"
#if SEPARATE_BUSY_TIMES
	     "  Read : %3d\n"
	     "  Write : %3d\n"
#endif
         "  Total : %3d"),
	     poConf->acTitle,
	     arPerf[R_DATA],
	     arPerf[W_DATA],
	     arPerf[RW_DATA],
	     '%',
#if SEPARATE_BUSY_TIMES
	     (oPerf.qlen >= 0) ?
	     (int) round(arBusy[R_DATA]) : -1,
	     (oPerf.qlen >= 0) ?
	     (int) round(arBusy[W_DATA]) : -1,
#endif
	     (oPerf.qlen >= 0) ? (int) round(arBusy[RW_DATA]) : -1);
    gtk_widget_set_tooltip_text(GTK_WIDGET(poMonitor->wEventBox), acToolTips);

    switch (poConf->eStatistics) {
	case BUSY_TIME:
	    prData = arBusy;
	    for (i = 0; i < NMONITORS; i++)
		prData[i] /= 100;
	    break;
	case IO_TRANSFER:
	default:
	    prData = arPerf;
	    for (i = 0; i < NMONITORS; i++)
		prData[i] /= poConf->iMaxXferMBperSec;
	    break;
    }
    for (i = 0; i < NMONITORS; i++) {
	pr = prData + i;
	if (*pr > 1)
	    *pr = 1;
	else if (*pr < 0)
	    *pr = 0;
    }
    UpdateProgressBars(p_poPlugin, prData[RW_DATA], prData[R_DATA], prData[W_DATA]);

    return (0);
}				/* DisplayPerf() */

	/**************************************************************/

static gboolean SetTimer (void *p_pvPlugin)
	/* Recurrently update the panel-docked monitor bars through a
	   timer */
{
    struct diskperf_t *poPlugin = (diskperf_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);

    DisplayPerf (poPlugin);
    
    if (timerNeedsUpdate) {
        g_source_remove (poPlugin->iTimerId);
        poPlugin->iTimerId = 0;
        timerNeedsUpdate = 0;
    }

    /* reduce the default tooltip timeout to be smaller than the update interval otherwise
     * we won't see tooltips on GTK 2.16 or newer */
    GtkSettings *settings;
    settings = gtk_settings_get_default();
    if (g_object_class_find_property(G_OBJECT_GET_CLASS(settings), "gtk-tooltip-timeout"))
        g_object_set(settings, "gtk-tooltip-timeout",
                     poConf->iPeriod_ms - 10, NULL);

    if (!poPlugin->iTimerId)
        poPlugin->iTimerId = g_timeout_add (poConf->iPeriod_ms,
					    (GSourceFunc) SetTimer, poPlugin);
    return TRUE;
}				/* SetTimer() */

	/**************************************************************/

static int SetSingleBarColor (struct diskperf_t *p_poPlugin, int p_iBar)
	/* Set the color of a single monitor bar */
{
    struct diskperf_t *poPlugin = p_poPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);
    Widget_t       *pwBar;

    pwBar = poMonitor->aoPerfBar[p_iBar].pwBar;
	gtk_widget_modify_bg(GTK_WIDGET(*pwBar),
						 GTK_STATE_PRELIGHT,
						 &poConf->aoColor[p_iBar]);
	gtk_widget_modify_bg(GTK_WIDGET(*pwBar),
						 GTK_STATE_SELECTED,
						 &poConf->aoColor[p_iBar]);
	gtk_widget_modify_base(GTK_WIDGET(*pwBar),
						 GTK_STATE_SELECTED,
						 &poConf->aoColor[p_iBar]);
    return (0);
}				/* SetSingleBarColor() */

	/**************************************************************/

static int SetMonitorBarColor (struct diskperf_t *p_poPlugin)
	/* Set the monitor bar colors */
{
    struct diskperf_t *poPlugin = p_poPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);

    if (poConf->fRW_DataCombined)
	SetSingleBarColor (p_poPlugin, RW_DATA);
    else {
	SetSingleBarColor (p_poPlugin, R_DATA);
	SetSingleBarColor (p_poPlugin, W_DATA);
    }
    return (0);
}				/* SetMonitorBarColor() */

	/**************************************************************/

static int ResetMonitorBar (struct diskperf_t *p_poPlugin)
	/* Set order (Read-Write or Write-Read) and colors */
{
    struct diskperf_t *poPlugin = p_poPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);

    poMonitor->aoPerfBar[R_DATA].pwBar =
	poMonitor->awProgressBar + (poConf->eMonitorBarOrder == WR_ORDER);
    poMonitor->aoPerfBar[W_DATA].pwBar =
	poMonitor->awProgressBar + (poConf->eMonitorBarOrder == RW_ORDER);
    poMonitor->aoPerfBar[RW_DATA].pwBar = poMonitor->awProgressBar + 0;

    SetMonitorBarColor (poPlugin);

    return (0);
}				/* ResetMonitorBar() */

	/**************************************************************/

static int CreateMonitorBars (struct diskperf_t *p_poPlugin,
			      GtkOrientation p_iOrientation)
	/* Create the panel progressive bars */
{
    struct diskperf_t *poPlugin = p_poPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);
    Widget_t       *pwBar;
    int             i;

    poMonitor->wBox = gtk_box_new (p_iOrientation, 0);
    gtk_widget_show (poMonitor->wBox);

    gtk_container_add (GTK_CONTAINER (poMonitor->wEventBox),
		       poMonitor->wBox);

    poMonitor->wTitle = gtk_label_new (poConf->acTitle);
    if (poConf->fTitleDisplayed)
	gtk_widget_show (poMonitor->wTitle);
    gtk_box_pack_start (GTK_BOX (poMonitor->wBox),
			GTK_WIDGET (poMonitor->wTitle), FALSE, FALSE, 2);

    for (i = 0; i < 2; i++) {
	pwBar = poMonitor->awProgressBar + i;
	*pwBar = GTK_WIDGET (gtk_progress_bar_new ());
	gtk_orientable_set_orientation (GTK_ORIENTABLE(*pwBar), !p_iOrientation);
	gtk_progress_bar_set_inverted (GTK_PROGRESS_BAR(*pwBar),
					   (p_iOrientation ==
					    GTK_ORIENTATION_HORIZONTAL));

	if ((i == 1) && poConf->fRW_DataCombined)
	    gtk_widget_hide (GTK_WIDGET (*pwBar));
	else
	    gtk_widget_show (GTK_WIDGET (*pwBar));
	gtk_box_pack_start (GTK_BOX (poMonitor->wBox),
			    GTK_WIDGET (*pwBar), FALSE, FALSE, 0);
    }

    ResetMonitorBar (poPlugin);

    return (0);
}				/* CreateMonitorBars() */

	/**************************************************************/

static diskperf_t *diskperf_create_control (XfcePanelPlugin *plugin)
	/* Plugin API */
	/* Create the diskperf */
{
    struct diskperf_t *poPlugin;
    struct param_t *poConf;
    struct monitor_t *poMonitor;
#if !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__sun__)
    struct stat     oStat;
    int             status;
#endif

    poPlugin = g_new (diskperf_t, 1);
    memset (poPlugin, 0, sizeof (diskperf_t));
    poConf = &(poPlugin->oConf.oParam);
    poMonitor = &(poPlugin->oMonitor);

    poPlugin->plugin = plugin;
    
#if defined(__NetBSD__) || defined(__OpenBSD__)
    strncpy (poConf->acDevice, "wd0", 64);
    strncpy (poConf->acTitle, "wd0", 16);
#elif defined(__FreeBSD__)
    strncpy (poConf->acDevice, "ada0", 64);
    strncpy (poConf->acTitle, "ada0", 16);
#elif defined(__sun__)
    strncpy (poConf->acDevice, "sd0", 64);
    strncpy (poConf->acTitle, "sd0", 16);
#else
    strncpy (poConf->acDevice, "/dev/sda", 64);
    status = stat (poConf->acDevice, &oStat);
    poConf->st_rdev = (status == -1 ? 0 : oStat.st_rdev);
    strncpy (poConf->acTitle, "sda", 16);
#endif

    poConf->fTitleDisplayed = 1;

    gdk_rgba_parse (poConf->aoColor + R_DATA, "#0000FF");
    gdk_rgba_parse (poConf->aoColor + W_DATA, "#FF0000");
    gdk_rgba_parse (poConf->aoColor + RW_DATA, "#00FF00");

    poConf->iMaxXferMBperSec = 40;
    poConf->fRW_DataCombined = 1;
    poConf->iPeriod_ms = 500;
    poConf->eStatistics = IO_TRANSFER;
    poConf->eMonitorBarOrder = RW_ORDER;
    poPlugin->iTimerId = 0;
    poPlugin->oMonitor.oPrevPerf.timestamp_ns = 0;

    poMonitor->wEventBox = gtk_event_box_new ();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(poMonitor->wEventBox), FALSE);
    gtk_event_box_set_above_child(GTK_EVENT_BOX(poMonitor->wEventBox), TRUE);
    gtk_widget_show (poMonitor->wEventBox);

    xfce_panel_plugin_add_action_widget (plugin, poMonitor->wEventBox);
    
    return poPlugin;
}				/* diskperf_create_control() */

	/**************************************************************/

static void diskperf_free (XfcePanelPlugin *plugin, diskperf_t *poPlugin)
	/* Plugin API */
{
    if (poPlugin->iTimerId)
	g_source_remove (poPlugin->iTimerId);
    g_free (poPlugin);
}				/* diskperf_free() */

	/**************************************************************/

	/* Configuration Keywords */
#define CONF_USE_LABEL		"UseLabel"
#define CONF_LABEL_TEXT		"Text"
#define CONF_DEVICE		"Device"
#define CONF_UPDATE_PERIOD	"UpdatePeriod"
#define CONF_STATISTICS		"Statistics"
#define CONF_XFER_RATE		"XferRate"
#define CONF_COMBINE_RW_DATA	"CombineRWdata"
#define CONF_MONITOR_BAR_ORDER	"MonitorBarOrder"
#define CONF_READ_COLOR		"ReadColor"
#define CONF_WRITE_COLOR	"WriteColor"
#define CONF_READ_WRITE_COLOR	"ReadWriteColor"

	/**************************************************************/

static void diskperf_read_config (XfcePanelPlugin *plugin, 
                                  diskperf_t *poPlugin)
	/* Plugin API */
	/* Executed when the panel is started - Read the configuration
	   previously stored in xml file */
{
    const char *value;
    char *file;
    XfceRc *rc;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);
    Widget_t       *pw2ndBar = poPlugin->oMonitor.awProgressBar + 1;
#if !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__sun__)
    struct stat     oStat;
    int             status;
#endif
    
    if (!(file = xfce_panel_plugin_lookup_rc_file (plugin)))
        return;
    
    rc = xfce_rc_simple_open (file, TRUE);
    g_free (file);

    if (!rc)
        return;
    
    if ((value = xfce_rc_read_entry (rc, (CONF_DEVICE), NULL))) {
        memset (poConf->acDevice, 0, sizeof (poConf->acDevice));
        strncpy (poConf->acDevice, value, sizeof (poConf->acDevice) - 1);
#if !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__sun__)
        status = stat (poConf->acDevice, &oStat);
        poConf->st_rdev = (status == -1 ? 0 : oStat.st_rdev);
#endif
    }

    poConf->fTitleDisplayed = 
        xfce_rc_read_int_entry (rc, (CONF_USE_LABEL), 1);

    if (poConf->fTitleDisplayed)
        gtk_widget_show (GTK_WIDGET (poMonitor->wTitle));
    else
        gtk_widget_hide (GTK_WIDGET (poMonitor->wTitle));

#ifdef HAS_PANEL_49
    /* unset small if we are in deskbar mode and we want the title */
    if (poConf->fTitleDisplayed && xfce_panel_plugin_get_mode(poPlugin->plugin) == XFCE_PANEL_PLUGIN_MODE_DESKBAR)
      xfce_panel_plugin_set_small (XFCE_PANEL_PLUGIN (poPlugin->plugin), FALSE);
    else
      xfce_panel_plugin_set_small (XFCE_PANEL_PLUGIN (poPlugin->plugin), TRUE);
#endif

    if ((value = xfce_rc_read_entry (rc, (CONF_LABEL_TEXT), NULL))) {
        memset (poConf->acTitle, 0, sizeof (poConf->acTitle));
        strncpy (poConf->acTitle, value, sizeof (poConf->acTitle) - 1);
        gtk_label_set_text (GTK_LABEL (poMonitor->wTitle),
                            poConf->acTitle);
    }

    poConf->iPeriod_ms = 
        xfce_rc_read_int_entry (rc, (CONF_UPDATE_PERIOD), 500);
    poConf->eStatistics = 
        xfce_rc_read_int_entry (rc, (CONF_STATISTICS), IO_TRANSFER);

    poConf->iMaxXferMBperSec = 
        xfce_rc_read_int_entry (rc, (CONF_XFER_RATE), 40);

    poConf->fRW_DataCombined = 
        xfce_rc_read_int_entry (rc, (CONF_COMBINE_RW_DATA), 1);

    if (poConf->fRW_DataCombined)
        gtk_widget_hide (GTK_WIDGET (*pw2ndBar));
    else
        gtk_widget_show (GTK_WIDGET (*pw2ndBar));

    poConf->eMonitorBarOrder = 
        xfce_rc_read_int_entry (rc, (CONF_MONITOR_BAR_ORDER), RW_ORDER);

    if ((value = xfce_rc_read_entry (rc, (CONF_READ_COLOR), NULL))) {
        gdk_rgba_parse (poConf->aoColor + R_DATA, value);
    }

    if ((value = xfce_rc_read_entry (rc, (CONF_WRITE_COLOR), NULL))) {
        gdk_rgba_parse (poConf->aoColor + W_DATA, value);
    }

    if ((value = xfce_rc_read_entry (rc, (CONF_READ_WRITE_COLOR), NULL))) {
        gdk_rgba_parse (poConf->aoColor + RW_DATA, value);
    }
    SetMonitorBarColor (poPlugin);

    xfce_rc_close (rc);
}				/* diskperf_read_config() */

	/**************************************************************/

static void diskperf_write_config (XfcePanelPlugin *plugin, 
                                   diskperf_t *poPlugin)
	/* Plugin API */
	/* Write diskperf configuration into xml file */
{
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    XfceRc *rc;
    char *file;

    if (!(file = xfce_panel_plugin_save_location (plugin, TRUE)))
        return;
    
    rc = xfce_rc_simple_open (file, FALSE);
    g_free (file);

    if (!rc)
        return;
    
    xfce_rc_write_entry (rc, CONF_DEVICE, poConf->acDevice);

    xfce_rc_write_int_entry (rc, CONF_USE_LABEL, poConf->fTitleDisplayed);

    xfce_rc_write_entry (rc, CONF_LABEL_TEXT, poConf->acTitle);

    xfce_rc_write_int_entry (rc, CONF_UPDATE_PERIOD, poConf->iPeriod_ms);

    xfce_rc_write_int_entry (rc, CONF_STATISTICS, poConf->eStatistics);

    xfce_rc_write_int_entry (rc, CONF_XFER_RATE, poConf->iMaxXferMBperSec);

    xfce_rc_write_int_entry (rc, CONF_COMBINE_RW_DATA, 
                             poConf->fRW_DataCombined);

    xfce_rc_write_int_entry (rc, CONF_MONITOR_BAR_ORDER, 
                             poConf->eMonitorBarOrder);

    xfce_rc_write_entry (rc, CONF_READ_COLOR, gdk_rgba_to_string(poConf->aoColor + R_DATA));
    xfce_rc_write_entry (rc, CONF_WRITE_COLOR, gdk_rgba_to_string(poConf->aoColor + W_DATA));
    xfce_rc_write_entry (rc, CONF_READ_WRITE_COLOR, gdk_rgba_to_string(poConf->aoColor + RW_DATA));

    xfce_rc_close (rc);
}				/* diskperf_write_config() */

	/**************************************************************/

static void SetDevice (Widget_t p_wTF, void *p_pvPlugin)
	/* GUI callback setting the device name, whose performance will be 
	   displayed using the panel-docked monitor bars */
{
    struct diskperf_t *poPlugin = (diskperf_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    const char     *pcDevice = gtk_entry_get_text (GTK_ENTRY (p_wTF));
#if !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__sun__)
    struct stat     oStat;
    int             status;

    status = stat (pcDevice, &oStat);
    poConf->st_rdev = oStat.st_rdev;
#endif
    memset (poConf->acDevice, 0, sizeof (poConf->acDevice));
    strncpy (poConf->acDevice, pcDevice, sizeof (poConf->acDevice) - 1);
}				/* SetDevice() */

	/**************************************************************/

static void ToggleTitle (Widget_t p_w, void *p_pvPlugin)
	/* GUI callback turning on/off the monitor bar legend */
{
    struct diskperf_t *poPlugin = (diskperf_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);

    poConf->fTitleDisplayed =
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (p_w));
    if (poConf->fTitleDisplayed)
	gtk_widget_show (GTK_WIDGET (poMonitor->wTitle));

    else
	gtk_widget_hide (GTK_WIDGET (poMonitor->wTitle));

#ifdef HAS_PANEL_49
    /* unset small if we are in deskbar mode and we want the title */
    if (poConf->fTitleDisplayed && xfce_panel_plugin_get_mode(poPlugin->plugin) == XFCE_PANEL_PLUGIN_MODE_DESKBAR)
      xfce_panel_plugin_set_small (XFCE_PANEL_PLUGIN (poPlugin->plugin), FALSE);
    else
      xfce_panel_plugin_set_small (XFCE_PANEL_PLUGIN (poPlugin->plugin), TRUE);
#endif
}				/* ToggleTitle() */

	/**************************************************************/

static void SetLabel (Widget_t p_wTF, void *p_pvPlugin)
	/* GUI callback setting the legend of the monitor bars */
{
    struct diskperf_t *poPlugin = (diskperf_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);
    const char     *acTitle = gtk_entry_get_text (GTK_ENTRY (p_wTF));

    memset (poConf->acTitle, 0, sizeof (poConf->acTitle));
    strncpy (poConf->acTitle, acTitle, sizeof (poConf->acTitle) - 1);
    gtk_label_set_text (GTK_LABEL (poMonitor->wTitle), poConf->acTitle);
}				/* SetLabel() */

	/**************************************************************/

static void ToggleStatistics (Widget_t p_w, void *p_pvPlugin)
	/* GUI callback allowing to choose statistics to monitor */
{
    struct diskperf_t *poPlugin = (diskperf_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    struct gui_t   *poGUI = &(poPlugin->oConf.oGUI);

    poConf->eStatistics = !(poConf->eStatistics);
    TRACE ("ToggleStatistics(): %d\n", poConf->eStatistics);
    switch (poConf->eStatistics) {
	case BUSY_TIME:
	    gtk_widget_hide (GTK_WIDGET (poGUI->wHBox_MaxIO));
	    if (!SEPARATE_BUSY_TIMES) {
		poConf->fRW_DataCombined = 1;
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (poGUI->wTB_RWcombined),
					      poConf->fRW_DataCombined);
	    }
	    break;
	case IO_TRANSFER:
	default:
	    gtk_widget_show (GTK_WIDGET (poGUI->wHBox_MaxIO));
    }
    gtk_widget_set_sensitive (GTK_WIDGET (poGUI->wTB_RWcombined),
			      (poConf->eStatistics != BUSY_TIME)
			      || SEPARATE_BUSY_TIMES);
}				/* ToggleStatistics() */

	/**************************************************************/

static void ToggleRWintegration (Widget_t p_w, void *p_pvPlugin)
	/* GUI callback allowing either to combine write + read data in a
	   single monitor bar, or to keep them separate using 2 dedicated
	   monitor bars */
{
    struct diskperf_t *poPlugin = (diskperf_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    struct gui_t   *poGUI = &(poPlugin->oConf.oGUI);
    Widget_t       *pw2ndBar = poPlugin->oMonitor.awProgressBar + 1;

    poConf->fRW_DataCombined =
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (p_w));
    TRACE ("ToggleRWintegration(): %d\n", poConf->fRW_DataCombined);
    if (poConf->fRW_DataCombined) {
	gtk_widget_hide (GTK_WIDGET (poGUI->wTa_DualBars));
	gtk_widget_show (GTK_WIDGET (poGUI->wTa_SingleBar));
	gtk_widget_hide (GTK_WIDGET (*pw2ndBar));
    }
    else {
	gtk_widget_hide (GTK_WIDGET (poGUI->wTa_SingleBar));
	gtk_widget_show (GTK_WIDGET (poGUI->wTa_DualBars));
	gtk_widget_show (GTK_WIDGET (*pw2ndBar));
    }
    SetMonitorBarColor (poPlugin);
}				/* ToggleRWintegration() */

	/**************************************************************/

static void ToggleRWorder (Widget_t p_w, void *p_pvPlugin)
	/* GUI callback allowing to swap Read/Write monitor bars */
{
    struct diskperf_t *poPlugin = (diskperf_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);

    poConf->eMonitorBarOrder = !(poConf->eMonitorBarOrder);
    TRACE ("ToggleRWorder(): %d\n", poConf->eMonitorBarOrder);
    ResetMonitorBar (poPlugin);
}				/* ToggleRWorder() */

	/**************************************************************/

static void SetXferRate (Widget_t p_wTF, void *p_pvPlugin)
	/* GUI callback setting the maximum I/O transfer rate of the
	   device */
{
    struct diskperf_t *poPlugin = (diskperf_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    const char     *pcXferRate = gtk_entry_get_text (GTK_ENTRY (p_wTF));

    /* Make it a multiple of 5 MB/s */
    poConf->iMaxXferMBperSec = 5 * round((double) atoi(pcXferRate) / 5);
    if (poConf->iMaxXferMBperSec > 995)
	poConf->iMaxXferMBperSec = 995;
    else if (poConf->iMaxXferMBperSec < 5)
	poConf->iMaxXferMBperSec = 5;
    DBG("XferRate rounded to %dMb/s\n", poConf->iMaxXferMBperSec);
}				/* SetXferRate() */

	/**************************************************************/

static void SetPeriod (Widget_t p_wSc, void *p_pvPlugin)
	/* Set the update period - To be used by the timer */
{
    struct diskperf_t *poPlugin = (diskperf_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    float           r;

    timerNeedsUpdate = 1;
    TRACE ("SetPeriod()\n");
    r = gtk_spin_button_get_value (GTK_SPIN_BUTTON (p_wSc));
    poConf->iPeriod_ms = round(r * 1000);
    DBG("Update period rounded to %dms\n", poConf->iPeriod_ms);
}				/* SetPeriod() */

	/**************************************************************/

static void ChooseColor (Widget_t p_wPB, void *p_pvPlugin)
{
    struct diskperf_t *poPlugin = (diskperf_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    struct gui_t   *poGUI = &(poPlugin->oConf.oGUI);
    GdkRGBA         poColor;
    int             iPerfBar;

    if (p_wPB == poGUI->wPB_Rcolor)
	iPerfBar = R_DATA;
    else if (p_wPB == poGUI->wPB_Wcolor)
	iPerfBar = W_DATA;
    else if (p_wPB == poGUI->wPB_RWcolor)
	iPerfBar = RW_DATA;
    else
	return;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(p_wPB), &poColor);
    DBG("color changed to %s for monitor %d", gdk_rgba_to_string(&poColor), iPerfBar);
    poConf->aoColor[iPerfBar] = poColor;
    SetMonitorBarColor (poPlugin);
}				/* ChooseColor() */

	/**************************************************************/

static void UpdateConf (diskperf_t *poPlugin)
	/* Called back when the configuration/options window is closed */
{
    struct conf_t  *poConf = &(poPlugin->oConf);
    struct gui_t   *poGUI = &(poConf->oGUI);

    TRACE ("UpdateConf()\n");
    SetDevice (poGUI->wTF_Device, poPlugin);
    SetLabel (poGUI->wTF_Title, poPlugin);
    SetXferRate (poGUI->wTF_MaxXfer, poPlugin);
    SetTimer (poPlugin);
}				/* UpdateConf() */

	/**************************************************************/

static int CheckStatsAvailability ()
	/* Check disk performance statistics availability */
	/* Return 0 on success, -1 otherwise */
{
    const char     *pcStatFile = 0;
    int             status;

    status = DevCheckStatAvailability (&pcStatFile);
    if (!status)
	return (0);
    if (status < 0) {
	status *= -1;
	xfce_dialog_show_error (NULL, NULL, 
                  _("%s\n"
		  "%s: %s (%d)\n\n"
		  "This monitor will not work!\n"
		  "Please remove it."),
		  PLUGIN_NAME, (pcStatFile ? pcStatFile : ""),
		  strerror (status), status);
	return (-1);
    }
    switch (status) {
	case NO_EXTENDED_STATS:
	    xfce_dialog_show_error (NULL, NULL, 
                      _("%s: No disk extended statistics found!\n"
		      "Either old kernel (< 2.4.20) or not\n"
		      "compiled with CONFIG_BLK_STATS turned on.\n\n"
		      "This monitor will not work!\n"
		      "Please remove it."), PLUGIN_NAME);
	    break;
	default:
	    xfce_dialog_show_error (NULL, NULL, 
                      _("%s: Unknown error\n\n"
		      "This monitor will not work!\n"
		      "Please remove it."), PLUGIN_NAME);
    }
    return (-1);
}				/* CheckStatsAvailability() */

	/**************************************************************/

static void About (Widget_t w, void *unused)
	/* Called back when the About button in clicked */
{
   GdkPixbuf *icon;
   const gchar *auth[] = { "Roger Seguin <roger_seguin@msn.com>",
	"NetBSD statistics collection: (c) 2003 Benedikt Meurer <benedikt.meurer@unix-ag.uni-siegen.de>",
	"Solaris statistics collection: (c) 2011 Peter Tribble <peter.tribble@gmail.com>", NULL };
   icon = xfce_panel_pixbuf_from_source("drive-harddisk", NULL, 32);
   gtk_show_about_dialog(NULL,
      "logo", icon,
      "license", xfce_get_license_text (XFCE_LICENSE_TEXT_BSD),
      "version", PACKAGE_VERSION,
      "program-name", PACKAGE_NAME,
      "comments", _("Diskperf monitor displays instantaneous disk I/O transfer rates and busy times"),
      "website", "http://goodies.xfce.org/projects/panel-plugins/xfce4-diskperf-plugin",
      "copyright", _("Copyright (c) 2003, 2004 Roger Seguin"),
      "authors", auth, NULL);
   if(icon)
      g_object_unref(G_OBJECT(icon));
}				/* About() */

	/**************************************************************/

static void diskperf_dialog_response (GtkWidget *dlg, int response, 
                                      diskperf_t *diskperf)
{
    UpdateConf (diskperf);
    gtk_widget_destroy (dlg);
    xfce_panel_plugin_unblock_menu (diskperf->plugin);
    diskperf_write_config (diskperf->plugin, diskperf);
}

static void diskperf_create_options (XfcePanelPlugin *plugin,
                                     diskperf_t *poPlugin)
	/* Plugin API */
	/* Create/pop up the configuration/options GUI */
{
    GtkWidget *dlg, *vbox;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    struct gui_t   *poGUI = &(poPlugin->oConf.oGUI);
    char            acBuffer[16];
    Widget_t       *apwColorPB[NMONITORS];
    int             i;

    TRACE ("diskperf_create_options()\n");

    (void) CheckStatsAvailability ();

    xfce_panel_plugin_block_menu (plugin);
    
    dlg = xfce_titled_dialog_new_with_buttons (_("Disk Performance Monitor"),
                                                GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (plugin))),
                                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                                "gtk-close", GTK_RESPONSE_OK,
                                                NULL);
    
    g_signal_connect (G_OBJECT (dlg), "response",
                      G_CALLBACK (diskperf_dialog_response), poPlugin);

    gtk_window_set_position (GTK_WINDOW (dlg), GTK_WIN_POS_CENTER);
    gtk_window_set_icon_name (GTK_WINDOW (dlg), "drive-harddisk");
    
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, BORDER);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), BORDER - 2);
    gtk_widget_show(vbox);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), vbox,
                        TRUE, TRUE, 0);
    
    poPlugin->oConf.wTopLevel = dlg;

    CreateConfigGUI (vbox, poGUI);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (poGUI->wTB_Title),
				  poConf->fTitleDisplayed);
    g_signal_connect (GTK_WIDGET (poGUI->wTB_Title), "toggled",
		      G_CALLBACK (ToggleTitle), poPlugin);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				  (poGUI->wRB_IO),
				  (poConf->eStatistics == IO_TRANSFER));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				  (poGUI->wRB_BusyTime),
				  (poConf->eStatistics == BUSY_TIME));
    if (poConf->eStatistics == IO_TRANSFER)
	gtk_widget_show (GTK_WIDGET (poGUI->wHBox_MaxIO));
    else {
	gtk_widget_hide (GTK_WIDGET (poGUI->wHBox_MaxIO));
	if (!SEPARATE_BUSY_TIMES)
	    poConf->fRW_DataCombined = 1;
    }
    g_signal_connect (GTK_WIDGET (poGUI->wRB_IO), "toggled",
		      G_CALLBACK (ToggleStatistics), poPlugin);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				  (poGUI->wTB_RWcombined),
				  poConf->fRW_DataCombined);
    gtk_widget_set_sensitive (GTK_WIDGET (poGUI->wTB_RWcombined),
			      (poConf->eStatistics != BUSY_TIME)
			      || SEPARATE_BUSY_TIMES);
    if (poConf->fRW_DataCombined) {
	gtk_widget_hide (GTK_WIDGET (poGUI->wTa_DualBars));
	gtk_widget_show (GTK_WIDGET (poGUI->wTa_SingleBar));
    }
    else {
	gtk_widget_hide (GTK_WIDGET (poGUI->wTa_SingleBar));
	gtk_widget_show (GTK_WIDGET (poGUI->wTa_DualBars));
    }
    g_signal_connect (GTK_WIDGET (poGUI->wTB_RWcombined), "toggled",
		      G_CALLBACK (ToggleRWintegration), poPlugin);

    gtk_entry_set_text (GTK_ENTRY (poGUI->wTF_Device), poConf->acDevice);
    g_signal_connect (GTK_WIDGET (poGUI->wTF_Device), "activate",
		      G_CALLBACK (SetDevice), poPlugin);

    gtk_entry_set_text (GTK_ENTRY (poGUI->wTF_Title), poConf->acTitle);
    g_signal_connect (GTK_WIDGET (poGUI->wTF_Title), "activate",
		      G_CALLBACK (SetLabel), poPlugin);

    snprintf (acBuffer, 16, "%d", poConf->iMaxXferMBperSec);
    gtk_entry_set_text (GTK_ENTRY (poGUI->wTF_MaxXfer), acBuffer);
    g_signal_connect (GTK_WIDGET (poGUI->wTF_MaxXfer), "activate",
		      G_CALLBACK (SetXferRate), poPlugin);

    gtk_spin_button_set_value (GTK_SPIN_BUTTON (poGUI->wSc_Period),
			       ((double) poConf->iPeriod_ms) / 1000);
    g_signal_connect (GTK_WIDGET (poGUI->wSc_Period), "value_changed",
		      G_CALLBACK (SetPeriod), poPlugin);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				  (poGUI->wRB_ReadWriteOrder),
				  (poConf->eMonitorBarOrder == RW_ORDER));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				  (poGUI->wRB_WriteReadOrder),
				  (poConf->eMonitorBarOrder == WR_ORDER));
    g_signal_connect (GTK_WIDGET (poGUI->wRB_ReadWriteOrder), "toggled",
		      G_CALLBACK (ToggleRWorder), poPlugin);

    apwColorPB[R_DATA] = &(poGUI->wPB_Rcolor);
    apwColorPB[W_DATA] = &(poGUI->wPB_Wcolor);
    apwColorPB[RW_DATA] = &(poGUI->wPB_RWcolor);
    for (i = 0; i < NMONITORS; i++) {
	poColorWidgets = poPlugin->oConf.aoColorWidgets + i;
	poColorWidgets->wDA = gtk_drawing_area_new ();
	gtk_widget_modify_bg (poColorWidgets->wDA, GTK_STATE_NORMAL,
			      poConf->aoColor + i);
	gtk_container_add (GTK_CONTAINER (*(apwColorPB[i])),
			   poColorWidgets->wDA);
	gtk_widget_show (GTK_WIDGET (poColorWidgets->wDA));
	g_signal_connect (GTK_WIDGET (*(apwColorPB[i])), "clicked",
			  G_CALLBACK (ChooseColor), poPlugin);
    }
		      
    gtk_widget_show (dlg);
}				/* diskperf_create_options() */

	/**************************************************************/

static gboolean diskperf_set_size (XfcePanelPlugin *plugin, int p_size,
                               diskperf_t *poPlugin)
	/* Plugin API */
	/* Set the size of the panel-docked monitor bars */
{
    int             i, size1, size2;
    Widget_t       *pwBar;
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);

    TRACE ("diskperf_set_size(%d)\n", p_size);
    gtk_container_set_border_width (GTK_CONTAINER
				    (poMonitor->wBox), p_size > 26 ? 2 : 1);
    if (xfce_panel_plugin_get_orientation (plugin) == 
            GTK_ORIENTATION_HORIZONTAL) {
	size1 = 8;
        size2 = -1;
        gtk_widget_set_size_request (GTK_WIDGET (plugin), -1, p_size);
    }
    else {
	size1 = -1;
        size2 = 8;
        gtk_widget_set_size_request (GTK_WIDGET (plugin), p_size, -1);
    }
    for (i = 0; i < 2; i++) {
	pwBar = poPlugin->oMonitor.awProgressBar + i;
	gtk_widget_set_size_request (GTK_WIDGET (*pwBar), size1, size2);
    }

    return TRUE;
}				/* diskperf_set_size() */

#ifdef HAS_PANEL_49
	/**************************************************************/

static void diskperf_set_mode (XfcePanelPlugin *plugin,
                               XfcePanelPluginMode p_iMode,
                               diskperf_t *poPlugin)
	/* Plugin API */
	/* Invoked when the panel changes mode */
{
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);
    GtkOrientation    p_iOrientation;

    TRACE ("diskperf_set_mode()\n");

    p_iOrientation =
      (p_iMode == XFCE_PANEL_PLUGIN_MODE_HORIZONTAL) ?
      GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL;


    if (poPlugin->iTimerId) {
	g_source_remove (poPlugin->iTimerId);
	poPlugin->iTimerId = 0;
    }
    gtk_container_remove (GTK_CONTAINER (poMonitor->wEventBox),
			  GTK_WIDGET (poMonitor->wBox));
    CreateMonitorBars (poPlugin, p_iOrientation);
    SetTimer (poPlugin);
    gtk_label_set_angle (GTK_LABEL (poMonitor->wTitle),
                         (p_iMode != XFCE_PANEL_PLUGIN_MODE_VERTICAL) ?
                         0 : 270);
    /* unset small if we are in deskbar mode and we want the title */
    if (poPlugin->oConf.oParam.fTitleDisplayed && p_iMode == XFCE_PANEL_PLUGIN_MODE_DESKBAR)
      xfce_panel_plugin_set_small (XFCE_PANEL_PLUGIN (plugin), FALSE);
    else
      xfce_panel_plugin_set_small (XFCE_PANEL_PLUGIN (plugin), TRUE);

    diskperf_set_size (plugin, xfce_panel_plugin_get_size (plugin), poPlugin);
}				/* diskperf_set_orientation() */
#else
	/**************************************************************/

static void diskperf_set_orientation (XfcePanelPlugin *plugin,
                                      GtkOrientation p_iOrientation,
                                      diskperf_t *poPlugin)
	/* Plugin API */
	/* Invoked when the panel changes orientation */
{
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);

    TRACE ("diskperf_set_orientation()\n");
    if (poPlugin->iTimerId) {
	g_source_remove (poPlugin->iTimerId);
	poPlugin->iTimerId = 0;
    }
    gtk_container_remove (GTK_CONTAINER (poMonitor->wEventBox),
			  GTK_WIDGET (poMonitor->wBox));
    CreateMonitorBars (poPlugin, p_iOrientation);
    SetTimer (poPlugin);
    diskperf_set_size (plugin, xfce_panel_plugin_get_size (plugin), poPlugin);
}				/* diskperf_set_orientation() */
#endif
	/**************************************************************/

static void diskperf_construct (XfcePanelPlugin *plugin)
{
    diskperf_t *diskperf = diskperf_create_control (plugin);
    xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

    g_signal_connect (plugin, "free-data", G_CALLBACK (diskperf_free), 
                      diskperf);

    g_signal_connect (plugin, "save", G_CALLBACK (diskperf_write_config), 
                      diskperf);

    g_signal_connect (plugin, "size-changed", G_CALLBACK (diskperf_set_size), 
                      diskperf);

#ifdef HAS_PANEL_49
    g_signal_connect (plugin, "mode-changed",
                      G_CALLBACK (diskperf_set_mode), diskperf);
    xfce_panel_plugin_set_small (plugin, TRUE);
#else
    g_signal_connect (plugin, "orientation-changed",
                      G_CALLBACK (diskperf_set_orientation), diskperf);
#endif

    xfce_panel_plugin_menu_show_about (plugin);
    g_signal_connect (plugin, "about", G_CALLBACK (About), NULL);

    xfce_panel_plugin_menu_show_configure (plugin);
    g_signal_connect (plugin, "configure-plugin", 
                      G_CALLBACK (diskperf_create_options), diskperf);

    gtk_container_add (GTK_CONTAINER (plugin), diskperf->oMonitor.wEventBox);

    CreateMonitorBars (diskperf, xfce_panel_plugin_get_orientation (plugin));
    
    diskperf_read_config (plugin, diskperf);
    DevPerfInit();
    
    SetTimer (diskperf);
}

XFCE_PANEL_PLUGIN_REGISTER (diskperf_construct);
