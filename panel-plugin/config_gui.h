/*
 * Copyright (c) 2003, 2004 Roger Seguin <roger_seguin@msn.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _config_gui_h
#define _config_gui_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/dialogs.h>


typedef struct gui_t {
    /* Configuration GUI widgets */
    GtkWidget      *wPB_About;
    GtkWidget      *wTF_Device;
    GtkWidget      *wTB_Title;
    GtkWidget      *wTF_Title;
    GtkWidget      *wSc_Period;
    GtkWidget      *wRB_IO;
    GtkWidget      *wRB_BusyTime;
    GtkWidget      *wHBox_MaxIO;
    GtkWidget      *wTF_MaxXfer;
    GtkWidget      *wTB_RWcombined;
    GtkWidget      *wTa_SingleBar;
    GtkWidget      *wTa_DualBars;
    GtkWidget      *wRB_ReadWriteOrder;
    GtkWidget      *wRB_WriteReadOrder;
    GtkWidget      *wPB_RWcolor;
    GtkWidget      *wPB_Rcolor;
    GtkWidget      *wPB_Wcolor;
} gui_t;


#ifdef __cplusplus
extern          "C" {
#endif

    int             CreateConfigGUI (GtkWidget * ParentWindow,
				     struct gui_t *gui);
    /* Create configuration/Option GUI */
    /* Return 0 on success, -1 otherwise */

#ifdef __cplusplus
}				/* extern "C" */
#endif
/*
$Log: config_gui.h,v $
Revision 1.3  2004/08/25 10:08:40  rogerms
DiskPerf 1.5

Revision 1.5  2004/08/25 08:50:08  RogerSeguin
Added About... dialog box

Revision 1.2  2003/11/04 10:26:13  rogerms
DiskPerf 1.3

Revision 1.4  2003/11/04 09:42:07  RogerSeguin
Added busy time statistics

Revision 1.1.1.1  2003/10/07 03:39:22  rogerms
Initial release - v1.0

Revision 1.3  2003/09/25 09:32:35  RogerSeguin
Added color configuration

Revision 1.2  2003/09/24 10:56:14  RogerSeguin
Now swapping the monitor bars is possible

Revision 1.1  2003/09/22 02:25:32  RogerSeguin
Initial revision

*/
#endif				/* _config_gui_h */
