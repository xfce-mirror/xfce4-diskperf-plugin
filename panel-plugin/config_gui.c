/* Copyright (c) 2003, 2004 Roger Seguin <roger_seguin@msn.com>
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


#include "config_gui.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>


#define COPYVAL(var, field)	((var)->field = field)


	/**** GUI initially created using glade-2 ****/

	/* Use the gtk_button_new_with_mnemonic() function for text-based
	   push buttons */
	/* Use "#define gtk_button_new_with_mnemonic(x) gtk_button_new()"
	   for color-filled buttons */

#define gtk_button_new_with_mnemonic(x) gtk_button_new()

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

int CreateConfigGUI (GtkWidget * vbox1, struct gui_t *p_poGUI)
{
    GtkWidget      *table1;
    GtkWidget      *label1;
    GtkWidget      *wTF_Device;
    GtkWidget      *eventbox1;
    GtkAdjustment  *wSc_Period_adj;
    GtkWidget      *wSc_Period;
    GtkWidget      *label2;
    GtkWidget      *wTB_Title;
    GtkWidget      *wTF_Title;
    GtkWidget      *hseparator7;
    GtkWidget      *hbox2;
    GtkWidget      *label9;
    GtkWidget      *wRB_IO;
    GSList         *wRB_IO_group = NULL;
    GtkWidget      *wRB_BusyTime;
    GtkWidget      *wHBox_MaxIO;
    GtkWidget      *label3;
    GtkWidget      *wTF_MaxXfer;
    GtkWidget      *hseparator8;
    GtkWidget      *wTB_RWcombined;
    GtkWidget      *wTa_SingleBar;
    GtkWidget      *label7;
    GtkWidget      *wPB_RWcolor;
    GtkWidget      *wTa_DualBars;
    GtkWidget      *label5;
    GtkWidget      *label6;
    GtkWidget      *label8;
    GtkWidget      *hbox1;
    GtkWidget      *wRB_ReadWriteOrder;
    GSList         *wRB_ReadWriteOrder_group = NULL;
    GtkWidget      *wRB_WriteReadOrder;
    GtkWidget      *wPB_Rcolor;
    GtkWidget      *wPB_Wcolor;

    table1 = gtk_grid_new ();
    gtk_grid_set_column_spacing(GTK_GRID (table1), 2);
    gtk_grid_set_row_spacing(GTK_GRID (table1), 2);
    gtk_widget_show (table1);
    gtk_box_pack_start (GTK_BOX (vbox1), table1, FALSE, FALSE, 0);

    label1 = gtk_label_new (_("Device"));
    gtk_widget_show (label1);
    gtk_grid_attach (GTK_GRID (table1), label1, 0, 0, 1, 1);
    gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);
    gtk_widget_set_valign(GTK_WIDGET (label1), GTK_ALIGN_CENTER);

    wTF_Device = gtk_entry_new ();
    gtk_widget_show (wTF_Device);
    gtk_grid_attach (GTK_GRID (table1), wTF_Device, 1, 0, 1, 1);
    gtk_widget_set_tooltip_text (wTF_Device,
			  _("Input the device name, then press <Enter>"));
    gtk_entry_set_max_length (GTK_ENTRY (wTF_Device), 64);
    gtk_entry_set_text (GTK_ENTRY (wTF_Device), _("/dev/sda1"));

    eventbox1 = gtk_event_box_new ();
    gtk_widget_show (eventbox1);
    gtk_grid_attach (GTK_GRID (table1), eventbox1, 1, 2, 1, 1);
    gtk_widget_set_valign(GTK_WIDGET(eventbox1), GTK_ALIGN_CENTER);
    gtk_widget_set_halign(GTK_WIDGET(eventbox1), GTK_ALIGN_CENTER);
    gtk_widget_set_vexpand(GTK_WIDGET(eventbox1), TRUE);
    gtk_widget_set_hexpand(GTK_WIDGET(eventbox1), TRUE);

    wSc_Period_adj = gtk_adjustment_new (0.5, 0.25, 4, 0.05, 1, 0);
    wSc_Period =
	gtk_spin_button_new (GTK_ADJUSTMENT (wSc_Period_adj), 1, 3);
    gtk_widget_show (wSc_Period);
    gtk_container_add (GTK_CONTAINER (eventbox1), wSc_Period);
    gtk_widget_set_tooltip_text (wSc_Period, _("Data collection period"));
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (wSc_Period), TRUE);

    label2 = gtk_label_new (_("Update interval (s) "));
    gtk_widget_show (label2);
    gtk_grid_attach (GTK_GRID (table1), label2, 0, 2, 1, 1);
    gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);
    gtk_widget_set_valign(GTK_WIDGET (label2), GTK_ALIGN_CENTER);

    wTB_Title = gtk_check_button_new_with_mnemonic (_("Label"));
    gtk_widget_show (wTB_Title);
    gtk_grid_attach (GTK_GRID (table1), wTB_Title, 0, 1, 1, 1);
    gtk_widget_set_tooltip_text (wTB_Title, _("Tick to display label"));

    wTF_Title = gtk_entry_new ();
    gtk_widget_show (wTF_Title);
    gtk_grid_attach (GTK_GRID (table1), wTF_Title, 1, 1, 1, 1);
    gtk_widget_set_tooltip_text (wTF_Title,
			  _("Input the label, then press <Enter>"));
    gtk_entry_set_max_length (GTK_ENTRY (wTF_Title), 16);
    gtk_entry_set_text (GTK_ENTRY (wTF_Title), _("hda1"));

    hseparator7 = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_show (hseparator7);
    gtk_box_pack_start (GTK_BOX (vbox1), hseparator7, TRUE, TRUE, 0);

    hbox2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_show (hbox2);
    gtk_box_pack_start (GTK_BOX (vbox1), hbox2, TRUE, TRUE, 0);

    label9 = gtk_label_new (_("Monitor    "));
    gtk_widget_show (label9);
    gtk_box_pack_start (GTK_BOX (hbox2), label9, FALSE, FALSE, 0);
    gtk_label_set_justify (GTK_LABEL (label9), GTK_JUSTIFY_LEFT);

    wRB_IO = gtk_radio_button_new_with_mnemonic (NULL, _("I/O transfer"));
    gtk_widget_show (wRB_IO);
    gtk_box_pack_start (GTK_BOX (hbox2), wRB_IO, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text (wRB_IO, _("MiB transferred / second"));
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (wRB_IO), wRB_IO_group);
    wRB_IO_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (wRB_IO));

    wRB_BusyTime =
	gtk_radio_button_new_with_mnemonic (NULL, _("Busy time"));
    gtk_widget_show (wRB_BusyTime);
    gtk_box_pack_start (GTK_BOX (hbox2), wRB_BusyTime, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text (wRB_BusyTime,
			  _("Percentage of time the device is busy"));
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (wRB_BusyTime),
				wRB_IO_group);
    wRB_IO_group =
	gtk_radio_button_get_group (GTK_RADIO_BUTTON (wRB_BusyTime));

    wHBox_MaxIO = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_show (wHBox_MaxIO);
    gtk_box_pack_start (GTK_BOX (vbox1), wHBox_MaxIO, TRUE, TRUE, 0);

    label3 = gtk_label_new (_("Max. I/O rate (MiB/s) "));
    gtk_widget_show (label3);
    gtk_box_pack_start (GTK_BOX (wHBox_MaxIO), label3, FALSE, FALSE, 0);
    gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);
    gtk_widget_set_valign (GTK_WIDGET (label3), GTK_ALIGN_CENTER);

    wTF_MaxXfer = gtk_entry_new ();
    gtk_widget_show (wTF_MaxXfer);
    gtk_box_pack_start (GTK_BOX (wHBox_MaxIO), wTF_MaxXfer, TRUE, TRUE, 0);
    gtk_widget_set_tooltip_text (wTF_MaxXfer,
			  _("Input the maximum I/O transfer rate of the device, then press <Enter>"));
    gtk_entry_set_max_length (GTK_ENTRY (wTF_MaxXfer), 3);
    gtk_entry_set_text (GTK_ENTRY (wTF_MaxXfer), _("35"));

    hseparator8 = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_show (hseparator8);
    gtk_box_pack_start (GTK_BOX (vbox1), hseparator8, FALSE, FALSE, 0);

    wTB_RWcombined =
	gtk_check_button_new_with_mnemonic (_("Combine Read/Write data"));
    gtk_widget_show (wTB_RWcombined);
    gtk_box_pack_start (GTK_BOX (vbox1), wTB_RWcombined, FALSE, FALSE, 0);
    gtk_widget_set_tooltip_text (wTB_RWcombined,
			  _("Combine Read/Write data into one single monitor?"));

    wTa_SingleBar = gtk_grid_new ();
    gtk_grid_set_column_spacing(GTK_GRID (wTa_SingleBar), 2);
    gtk_grid_set_row_spacing(GTK_GRID (wTa_SingleBar), 2);
    gtk_widget_show (wTa_SingleBar);
    gtk_box_pack_start (GTK_BOX (vbox1), wTa_SingleBar, FALSE, FALSE, 0);

    label7 = gtk_label_new (_("Bar color "));
    gtk_widget_show (label7);
    gtk_grid_attach (GTK_GRID (wTa_SingleBar), label7, 0, 0, 1, 1);
    gtk_label_set_justify (GTK_LABEL (label7), GTK_JUSTIFY_LEFT);
    gtk_widget_set_valign (GTK_WIDGET (label7), GTK_ALIGN_CENTER);

    wPB_RWcolor = gtk_button_new_with_mnemonic ("");
    gtk_widget_show (wPB_RWcolor);
    gtk_grid_attach (GTK_GRID (wTa_SingleBar), wPB_RWcolor, 1, 0, 1, 1);
    gtk_widget_set_tooltip_text (wPB_RWcolor, _("Press to change color"));

    wTa_DualBars = gtk_grid_new ();
    gtk_grid_set_column_spacing(GTK_GRID (wTa_DualBars), 2);
    gtk_grid_set_row_spacing(GTK_GRID (wTa_DualBars), 2);
    gtk_widget_show (wTa_DualBars);
    gtk_box_pack_start (GTK_BOX (vbox1), wTa_DualBars, FALSE, FALSE, 0);

    label5 = gtk_label_new (_("Read bar color "));
    gtk_widget_show (label5);
    gtk_grid_attach (GTK_GRID (wTa_DualBars), label5, 0, 1, 1, 1);
    gtk_label_set_justify (GTK_LABEL (label5), GTK_JUSTIFY_LEFT);
    gtk_widget_set_valign (GTK_WIDGET (label5), GTK_ALIGN_CENTER);

    label6 = gtk_label_new (_("Write bar color "));
    gtk_widget_show (label6);
    gtk_grid_attach (GTK_GRID (wTa_DualBars), label6, 0, 2, 1, 1);
    gtk_label_set_justify (GTK_LABEL (label6), GTK_JUSTIFY_LEFT);
    gtk_widget_set_valign (GTK_WIDGET (label6), GTK_ALIGN_CENTER);

    label8 = gtk_label_new (_("Bar order"));
    gtk_widget_show (label8);
    gtk_grid_attach (GTK_GRID (wTa_DualBars), label8, 0, 0, 1, 1);
    gtk_label_set_justify (GTK_LABEL (label8), GTK_JUSTIFY_LEFT);
    gtk_widget_set_valign (GTK_WIDGET (label8), GTK_ALIGN_CENTER);

    hbox1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_show (hbox1);
    gtk_grid_attach (GTK_GRID (wTa_DualBars), hbox1, 1, 0, 1, 1);

    wRB_ReadWriteOrder =
	gtk_radio_button_new_with_mnemonic (NULL, _("Read-Write"));
    gtk_widget_show (wRB_ReadWriteOrder);
    gtk_box_pack_start (GTK_BOX (hbox1), wRB_ReadWriteOrder, FALSE, FALSE,
			0);
    gtk_widget_set_tooltip_text (wRB_ReadWriteOrder,
			  _("\"Read\" monitor first"));
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (wRB_ReadWriteOrder),
				wRB_ReadWriteOrder_group);
    wRB_ReadWriteOrder_group =
	gtk_radio_button_get_group (GTK_RADIO_BUTTON (wRB_ReadWriteOrder));

    wRB_WriteReadOrder =
	gtk_radio_button_new_with_mnemonic (NULL, _("Write-Read"));
    gtk_widget_show (wRB_WriteReadOrder);
    gtk_box_pack_start (GTK_BOX (hbox1), wRB_WriteReadOrder, FALSE, FALSE,
			0);
    gtk_widget_set_tooltip_text (wRB_WriteReadOrder,
			  _("\"Write\" monitor first"));
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (wRB_WriteReadOrder),
				wRB_ReadWriteOrder_group);
    wRB_ReadWriteOrder_group =
	gtk_radio_button_get_group (GTK_RADIO_BUTTON (wRB_WriteReadOrder));

    wPB_Rcolor = gtk_button_new_with_mnemonic ("");
    gtk_widget_show (wPB_Rcolor);
    gtk_grid_attach (GTK_GRID (wTa_DualBars), wPB_Rcolor, 1, 1, 1, 1);
    gtk_widget_set_tooltip_text (wPB_Rcolor, _("Press to change color"));

    wPB_Wcolor = gtk_button_new_with_mnemonic ("");
    gtk_widget_show (wPB_Wcolor);
    gtk_grid_attach (GTK_GRID (wTa_DualBars), wPB_Wcolor, 1, 2, 1, 1);
    gtk_widget_set_tooltip_text (wPB_Wcolor, _("Press to change color"));

    if (p_poGUI) {
	COPYVAL (p_poGUI, wTF_Device);
	COPYVAL (p_poGUI, wSc_Period);
	COPYVAL (p_poGUI, wTB_Title);
	COPYVAL (p_poGUI, wTF_Title);
	COPYVAL (p_poGUI, wRB_IO);
	COPYVAL (p_poGUI, wRB_BusyTime);
	COPYVAL (p_poGUI, wHBox_MaxIO);
	COPYVAL (p_poGUI, wTF_MaxXfer);
	COPYVAL (p_poGUI, wTB_RWcombined);
	COPYVAL (p_poGUI, wTa_SingleBar);
	COPYVAL (p_poGUI, wTa_DualBars);
	COPYVAL (p_poGUI, wRB_ReadWriteOrder);
	COPYVAL (p_poGUI, wRB_WriteReadOrder);
	COPYVAL (p_poGUI, wPB_RWcolor);
	COPYVAL (p_poGUI, wPB_Rcolor);
	COPYVAL (p_poGUI, wPB_Wcolor);
    }
    return (0);
}				/* CreateConfigGUI() */
