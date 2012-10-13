/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Panel */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <Desktop.h>
#include "../include/Panel.h"


/* private */
/* prototypes */
static int _message(unsigned int timeout, GtkMessageType type,
		char const * message);

/* callbacks */
static gboolean _message_on_timeout(gpointer data);

static int _usage(void);


/* functions */
/* message */
static int _message(unsigned int timeout, GtkMessageType type,
		char const * message)
{
	PangoFontDescription * bold;
	char const * title;
	char const * stock;
	GtkWidget * plug;
	GtkWidget * hbox;
	GtkWidget * vbox;
	GtkWidget * image;
	GtkWidget * widget;
	uint32_t xid;

	bold = pango_font_description_new();
	pango_font_description_set_weight(bold, PANGO_WEIGHT_BOLD);
	switch(type)
	{
		case GTK_MESSAGE_ERROR:
			stock = GTK_STOCK_DIALOG_ERROR;
			title = "Error";
			break;
		case GTK_MESSAGE_QUESTION:
			stock = GTK_STOCK_DIALOG_QUESTION;
			title = "Question";
			break;
		case GTK_MESSAGE_WARNING:
			stock = GTK_STOCK_DIALOG_WARNING;
			title = "Warning";
			break;
		case GTK_MESSAGE_INFO:
		default:
			stock = GTK_STOCK_DIALOG_INFO;
			title = "Information";
			break;
	}
	plug = gtk_plug_new(0);
	gtk_container_set_border_width(GTK_CONTAINER(plug), 4);
	hbox = gtk_hbox_new(FALSE, 4);
	/* icon */
	widget = gtk_image_new_from_stock(stock, GTK_ICON_SIZE_DIALOG);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	/* title */
	vbox = gtk_vbox_new(FALSE, 4);
	widget = gtk_label_new(title);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.0);
	gtk_widget_modify_font(widget, bold);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	/* label */
	widget = gtk_label_new(message);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.0);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
	/* button */
	widget = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
	image = gtk_image_new_from_stock(GTK_STOCK_CLOSE,
			GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(widget), image);
	g_signal_connect(widget, "clicked", G_CALLBACK(gtk_main_quit), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(plug), hbox);
	gtk_widget_show_all(plug);
	xid = gtk_plug_get_id(GTK_PLUG(plug));
	desktop_message_send(PANEL_CLIENT_MESSAGE, PANEL_MESSAGE_EMBED, xid, 0);
	if(timeout > 0)
		g_timeout_add(timeout * 1000, _message_on_timeout, NULL);
	pango_font_description_free(bold);
	gtk_main();
	return 0;
}


/* callbacks */
/* message_on_timeout */
static gboolean _message_on_timeout(gpointer data)
{
	gtk_main_quit();
	return FALSE;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: panel-message [-EIQW][-t timeout] message\n", stderr);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	GtkMessageType type = GTK_MESSAGE_INFO;
	unsigned int timeout = 3;
	int o;
	char * p;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "EIQWt:")) != -1)
		switch(o)
		{
			case 'E':
				type = GTK_MESSAGE_ERROR;
				break;
			case 'I':
				type = GTK_MESSAGE_INFO;
				break;
			case 'Q':
				type = GTK_MESSAGE_QUESTION;
				break;
			case 'W':
				type = GTK_MESSAGE_WARNING;
				break;
			case 't':
				timeout = strtoul(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0')
					return _usage();
				break;
			default:
				return _usage();
		}
	if(argc - optind != 1)
		return _usage();
	return (_message(timeout, type, argv[optind]) == 0) ? 0 : 2;
}
