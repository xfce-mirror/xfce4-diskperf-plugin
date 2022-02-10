/*
 * This file is part of Xfce (https://gitlab.xfce.org).
 *
 * Copyright (c) 2003-2004 Roger Seguin <roger_seguin@msn.com>
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

#ifndef _config_gui_h
#define _config_gui_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4util/libxfce4util.h>

typedef struct gui_t {
    /* Configuration GUI widgets */
    GtkWidget *wTF_Device;
    GtkWidget *wTB_Title;
    GtkWidget *wTF_Title;
    GtkWidget *wSc_Period;
    GtkWidget *wRB_IO;
    GtkWidget *wRB_BusyTime;
    GtkWidget *wHBox_MaxIO;
    GtkWidget *wTF_MaxXfer;
    GtkWidget *wTB_RWcombined;
    GtkWidget *wTa_SingleBar;
    GtkWidget *wTa_DualBars;
    GtkWidget *wRB_ReadWriteOrder;
    GtkWidget *wRB_WriteReadOrder;
    GtkWidget *wPB_RWcolor;
    GtkWidget *wPB_Rcolor;
    GtkWidget *wPB_Wcolor;
} gui_t;

G_BEGIN_DECLS

/* Create configuration/Option GUI */
/* Return 0 on success, -1 otherwise */
int CreateConfigGUI (GtkWidget * ParentWindow, struct gui_t *gui);

G_END_DECLS

#endif /* _config_gui_h */
