/*
 * Copyright (c) 2003 RogerSeguin <roger_seguin@msn.com>
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


static char     _config_gui_id[] =
    "$Id: config_gui.c,v 1.1 2003/10/07 00:56:42 rogerms Exp $";


#include "config_gui.h"


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include <libxfce4util/i18n.h>
#include <libxfcegui4/dialogs.h>
#include <panel/plugins.h>
#include <panel/xfce.h>


#define COPYVAL(var, field)	((var)->field = field)

	/* GUI initially created using glade-2 */
#if 0
GtkWidget      *create_OptionDialog (void)
#else

#define gtk_button_new_with_mnemonic(x)	gtk_button_new ()

int CreateConfigGUI (GtkWidget * OptionDialog, struct gui_t *p_poGUI)
#endif
{
#if 0
    GtkWidget      *OptionDialog;
#endif

    GtkWidget      *vbox1;
    GtkWidget      *table1;
    GtkWidget      *label1;
    GtkWidget      *wTF_Device;
    GtkWidget      *eventbox1;
    GtkWidget      *alignment1;
    GtkObject      *wSc_Period_adj;
    GtkWidget      *wSc_Period;
    GtkWidget      *label2;
    GtkWidget      *wTB_Title;
    GtkWidget      *wTF_Title;
    GtkWidget      *label3;
    GtkWidget      *wTF_MaxXfer;
    GtkWidget      *wTB_RWcombined;
    GtkWidget      *hseparator2;
    GtkWidget      *wTa_SingleBar;
    GtkWidget      *label7;
    GtkWidget      *wPB_RWcolor;
    GtkWidget      *wTa_DualBars;
    GtkWidget      *label5;
    GtkWidget      *label6;
    GtkWidget      *label8;
    GtkWidget      *hbox1;
    GtkWidget      *wTB_ReadWriteOrder;
    GSList         *wTB_ReadWriteOrder_group = NULL;
    GtkWidget      *wTB_WriteReadOrder;
    GtkWidget      *wPB_Rcolor;
    GtkWidget      *wPB_Wcolor;
    GtkTooltips    *tooltips;

    tooltips = gtk_tooltips_new ();

#if 0
    OptionDialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (OptionDialog), _("OptionDialog"));
#endif

    vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox1);
    gtk_container_add (GTK_CONTAINER (OptionDialog), vbox1);

    table1 = gtk_table_new (4, 2, FALSE);
    gtk_widget_show (table1);
    gtk_box_pack_start (GTK_BOX (vbox1), table1, FALSE, TRUE, 0);

    label1 = gtk_label_new (_("Device"));
    gtk_widget_show (label1);
    gtk_table_attach (GTK_TABLE (table1), label1, 0, 1, 0, 1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);

    wTF_Device = gtk_entry_new ();
    gtk_widget_show (wTF_Device);
    gtk_table_attach (GTK_TABLE (table1), wTF_Device, 1, 2, 0, 1,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_tooltips_set_tip (tooltips, wTF_Device,
			  _("Input the device name, then press <Enter>"),
			  NULL);
    gtk_entry_set_max_length (GTK_ENTRY (wTF_Device), 64);
    gtk_entry_set_text (GTK_ENTRY (wTF_Device), _("/dev/hda1"));

    eventbox1 = gtk_event_box_new ();
    gtk_widget_show (eventbox1);
    gtk_table_attach (GTK_TABLE (table1), eventbox1, 1, 2, 2, 3,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 0, 0);

    alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment1);
    gtk_container_add (GTK_CONTAINER (eventbox1), alignment1);

    wSc_Period_adj = gtk_adjustment_new (0.5, 0.25, 4, 0.05, 1, 10);
    wSc_Period =
	gtk_spin_button_new (GTK_ADJUSTMENT (wSc_Period_adj), 1, 3);
    gtk_widget_show (wSc_Period);
    gtk_container_add (GTK_CONTAINER (alignment1), wSc_Period);
    gtk_tooltips_set_tip (tooltips, wSc_Period,
			  _("Data collection period"), NULL);
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (wSc_Period), TRUE);

    label2 = gtk_label_new (_("Update interval (s) "));
    gtk_widget_show (label2);
    gtk_table_attach (GTK_TABLE (table1), label2, 0, 1, 2, 3,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);

    wTB_Title = gtk_check_button_new_with_mnemonic (_("Label"));
    gtk_widget_show (wTB_Title);
    gtk_table_attach (GTK_TABLE (table1), wTB_Title, 0, 1, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_tooltips_set_tip (tooltips, wTB_Title, _("Tick to display label"),
			  NULL);

    wTF_Title = gtk_entry_new ();
    gtk_widget_show (wTF_Title);
    gtk_table_attach (GTK_TABLE (table1), wTF_Title, 1, 2, 1, 2,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_tooltips_set_tip (tooltips, wTF_Title,
			  _("Input the label, then press <Enter>"), NULL);
    gtk_entry_set_max_length (GTK_ENTRY (wTF_Title), 16);
    gtk_entry_set_text (GTK_ENTRY (wTF_Title), _("hda1"));

    label3 = gtk_label_new (_("Max. I/O rate (MB/s) "));
    gtk_widget_show (label3);
    gtk_table_attach (GTK_TABLE (table1), label3, 0, 1, 3, 4,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);

    wTF_MaxXfer = gtk_entry_new ();
    gtk_widget_show (wTF_MaxXfer);
    gtk_table_attach (GTK_TABLE (table1), wTF_MaxXfer, 1, 2, 3, 4,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_tooltips_set_tip (tooltips, wTF_MaxXfer,
			  _
			  ("Input the maximum I/O transfer rate of the device, then press <Enter>"),
			  NULL);
    gtk_entry_set_max_length (GTK_ENTRY (wTF_MaxXfer), 3);
    gtk_entry_set_text (GTK_ENTRY (wTF_MaxXfer), _("35"));

    wTB_RWcombined =
	gtk_check_button_new_with_mnemonic (_("Combine Read/Write data"));
    gtk_widget_show (wTB_RWcombined);
    gtk_box_pack_start (GTK_BOX (vbox1), wTB_RWcombined, FALSE, FALSE, 0);
    gtk_tooltips_set_tip (tooltips, wTB_RWcombined,
			  _
			  ("Combine Read/Write data into one single monitor?"),
			  NULL);

    hseparator2 = gtk_hseparator_new ();
    gtk_widget_show (hseparator2);
    gtk_box_pack_start (GTK_BOX (vbox1), hseparator2, TRUE, TRUE, 0);

    wTa_SingleBar = gtk_table_new (1, 2, FALSE);
    gtk_widget_show (wTa_SingleBar);
    gtk_box_pack_start (GTK_BOX (vbox1), wTa_SingleBar, TRUE, TRUE, 0);

    label7 = gtk_label_new (_("Bar color "));
    gtk_widget_show (label7);
    gtk_table_attach (GTK_TABLE (wTa_SingleBar), label7, 0, 1, 0, 1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label7), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (label7), 0, 0.5);

    wPB_RWcolor = gtk_button_new_with_mnemonic ("");
    gtk_widget_show (wPB_RWcolor);
    gtk_table_attach (GTK_TABLE (wTa_SingleBar), wPB_RWcolor, 1, 2, 0, 1,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (GTK_EXPAND), 0, 0);
    gtk_tooltips_set_tip (tooltips, wPB_RWcolor,
			  _("Press to change color"), NULL);

    wTa_DualBars = gtk_table_new (3, 2, FALSE);
    gtk_widget_show (wTa_DualBars);
    gtk_box_pack_start (GTK_BOX (vbox1), wTa_DualBars, TRUE, TRUE, 0);

    label5 = gtk_label_new (_("Read bar color "));
    gtk_widget_show (label5);
    gtk_table_attach (GTK_TABLE (wTa_DualBars), label5, 0, 1, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label5), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (label5), 0, 0.5);

    label6 = gtk_label_new (_("Write  bar color "));
    gtk_widget_show (label6);
    gtk_table_attach (GTK_TABLE (wTa_DualBars), label6, 0, 1, 2, 3,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label6), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (label6), 0, 0.5);

    label8 = gtk_label_new (_("Bar order"));
    gtk_widget_show (label8);
    gtk_table_attach (GTK_TABLE (wTa_DualBars), label8, 0, 1, 0, 1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label8), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (label8), 0, 0.5);

    hbox1 = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox1);
    gtk_table_attach (GTK_TABLE (wTa_DualBars), hbox1, 1, 2, 0, 1,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 0, 0);

    wTB_ReadWriteOrder =
	gtk_radio_button_new_with_mnemonic (NULL, _("Read-Write"));
    gtk_widget_show (wTB_ReadWriteOrder);
    gtk_box_pack_start (GTK_BOX (hbox1), wTB_ReadWriteOrder, FALSE, FALSE,
			0);
    gtk_tooltips_set_tip (tooltips, wTB_ReadWriteOrder,
			  _("\"Read\" monitor first"), NULL);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (wTB_ReadWriteOrder),
				wTB_ReadWriteOrder_group);
    wTB_ReadWriteOrder_group =
	gtk_radio_button_get_group (GTK_RADIO_BUTTON (wTB_ReadWriteOrder));

    wTB_WriteReadOrder =
	gtk_radio_button_new_with_mnemonic (NULL, _("Write-Read"));
    gtk_widget_show (wTB_WriteReadOrder);
    gtk_box_pack_start (GTK_BOX (hbox1), wTB_WriteReadOrder, FALSE, FALSE,
			0);
    gtk_tooltips_set_tip (tooltips, wTB_WriteReadOrder,
			  _("\"Write\" monitor first"), NULL);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (wTB_WriteReadOrder),
				wTB_ReadWriteOrder_group);
    wTB_ReadWriteOrder_group =
	gtk_radio_button_get_group (GTK_RADIO_BUTTON (wTB_WriteReadOrder));

    wPB_Rcolor = gtk_button_new_with_mnemonic ("");
    gtk_widget_show (wPB_Rcolor);
    gtk_table_attach (GTK_TABLE (wTa_DualBars), wPB_Rcolor, 1, 2, 1, 2,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (GTK_EXPAND), 0, 0);
    gtk_tooltips_set_tip (tooltips, wPB_Rcolor, _("Press to change color"),
			  NULL);

    wPB_Wcolor = gtk_button_new_with_mnemonic ("");
    gtk_widget_show (wPB_Wcolor);
    gtk_table_attach (GTK_TABLE (wTa_DualBars), wPB_Wcolor, 1, 2, 2, 3,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (GTK_EXPAND), 0, 0);
    gtk_tooltips_set_tip (tooltips, wPB_Wcolor, _("Press to change color"),
			  NULL);

#if 0
    /* Store pointers to all widgets, for use by lookup_widget(). */
    GLADE_HOOKUP_OBJECT_NO_REF (OptionDialog, OptionDialog,
				"OptionDialog");
    GLADE_HOOKUP_OBJECT (OptionDialog, vbox1, "vbox1");
    GLADE_HOOKUP_OBJECT (OptionDialog, table1, "table1");
    GLADE_HOOKUP_OBJECT (OptionDialog, label1, "label1");
    GLADE_HOOKUP_OBJECT (OptionDialog, wTF_Device, "wTF_Device");
    GLADE_HOOKUP_OBJECT (OptionDialog, eventbox1, "eventbox1");
    GLADE_HOOKUP_OBJECT (OptionDialog, alignment1, "alignment1");
    GLADE_HOOKUP_OBJECT (OptionDialog, wSc_Period, "wSc_Period");
    GLADE_HOOKUP_OBJECT (OptionDialog, label2, "label2");
    GLADE_HOOKUP_OBJECT (OptionDialog, wTB_Title, "wTB_Title");
    GLADE_HOOKUP_OBJECT (OptionDialog, wTF_Title, "wTF_Title");
    GLADE_HOOKUP_OBJECT (OptionDialog, label3, "label3");
    GLADE_HOOKUP_OBJECT (OptionDialog, wTF_MaxXfer, "wTF_MaxXfer");
    GLADE_HOOKUP_OBJECT (OptionDialog, wTB_RWcombined, "wTB_RWcombined");
    GLADE_HOOKUP_OBJECT (OptionDialog, hseparator2, "hseparator2");
    GLADE_HOOKUP_OBJECT (OptionDialog, wTa_SingleBar, "wTa_SingleBar");
    GLADE_HOOKUP_OBJECT (OptionDialog, label7, "label7");
    GLADE_HOOKUP_OBJECT (OptionDialog, wPB_RWcolor, "wPB_RWcolor");
    GLADE_HOOKUP_OBJECT (OptionDialog, wTa_DualBars, "wTa_DualBars");
    GLADE_HOOKUP_OBJECT (OptionDialog, label5, "label5");
    GLADE_HOOKUP_OBJECT (OptionDialog, label6, "label6");
    GLADE_HOOKUP_OBJECT (OptionDialog, label8, "label8");
    GLADE_HOOKUP_OBJECT (OptionDialog, hbox1, "hbox1");
    GLADE_HOOKUP_OBJECT (OptionDialog, wTB_ReadWriteOrder,
			 "wTB_ReadWriteOrder");
    GLADE_HOOKUP_OBJECT (OptionDialog, wTB_WriteReadOrder,
			 "wTB_WriteReadOrder");
    GLADE_HOOKUP_OBJECT (OptionDialog, wPB_Rcolor, "wPB_Rcolor");
    GLADE_HOOKUP_OBJECT (OptionDialog, wPB_Wcolor, "wPB_Wcolor");
    GLADE_HOOKUP_OBJECT_NO_REF (OptionDialog, tooltips, "tooltips");

    return OptionDialog;

#else
    if (p_poGUI) {
	COPYVAL (p_poGUI, wTF_Device);
	COPYVAL (p_poGUI, wSc_Period);
	COPYVAL (p_poGUI, wTB_Title);
	COPYVAL (p_poGUI, wTF_Title);
	COPYVAL (p_poGUI, wTF_MaxXfer);
	COPYVAL (p_poGUI, wTB_RWcombined);
	COPYVAL (p_poGUI, wTa_SingleBar);
	COPYVAL (p_poGUI, wTa_DualBars);
	COPYVAL (p_poGUI, wTB_ReadWriteOrder);
	COPYVAL (p_poGUI, wTB_WriteReadOrder);
	COPYVAL (p_poGUI, wPB_RWcolor);
	COPYVAL (p_poGUI, wPB_Rcolor);
	COPYVAL (p_poGUI, wPB_Wcolor);
    }
    return (0);
#endif
}				/* CreateConfigGUI() */


/*
$Log: config_gui.c,v $
Revision 1.1  2003/10/07 00:56:42  rogerms
Initial revision

Revision 1.4  2003/09/25 09:32:52  RogerSeguin
Added color configuration

Revision 1.3  2003/09/24 10:56:24  RogerSeguin
Now swapping the monitor bars is possible

Revision 1.2  2003/09/23 11:21:11  RogerSeguin
Slight change

Revision 1.1  2003/09/22 02:25:31  RogerSeguin
Initial revision

*/
