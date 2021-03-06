/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
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



#include <System.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include "Browser.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* Preview */
/* private */
/* types */
typedef struct _BrowserPlugin
{
	BrowserPluginHelper * helper;

	char * path;
	guint source;

	/* widgets */
	GtkWidget * widget;
	GtkWidget * name;
	GtkWidget * open;
	GtkWidget * edit;
	GtkWidget * view_image;
	GtkWidget * view_text;
	GtkTextBuffer * view_text_buffer;
} Preview;


/* prototypes */
/* plug-in */
static Preview * _preview_init(BrowserPluginHelper * helper);
static void _preview_destroy(Preview * preview);
static GtkWidget * _preview_get_widget(Preview * preview);
static void _preview_refresh(Preview * preview, GList * selection);

/* callbacks */
static void _preview_on_edit(gpointer data);
static gboolean _preview_on_idle_image(gpointer data);
static gboolean _preview_on_idle_text(gpointer data);
static void _preview_on_open(gpointer data);


/* public */
/* variables */
BrowserPluginDefinition plugin =
{
	N_("Preview"),
	NULL,
	NULL,
	_preview_init,
	_preview_destroy,
	_preview_get_widget,
	_preview_refresh
};


/* private */
/* functions */
/* preview_init */
static Preview * _preview_init(BrowserPluginHelper * helper)
{
	Preview * preview;
	PangoFontDescription * font;
	GtkSizeGroup * group;
	GtkWidget * vbox;
	GtkWidget * widget;

	if((preview = object_new(sizeof(*preview))) == NULL)
		return NULL;
	preview->helper = helper;
	preview->path = NULL;
	preview->source = 0;
	/* widgets */
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	vbox = gtk_vbox_new(FALSE, 4);
	preview->widget = vbox;
	/* name */
	preview->name = gtk_label_new(NULL);
	gtk_label_set_ellipsize(GTK_LABEL(preview->name),
			PANGO_ELLIPSIZE_MIDDLE);
	gtk_misc_set_alignment(GTK_MISC(preview->name), 0.0, 0.5);
	font = pango_font_description_new();
	pango_font_description_set_weight(font, PANGO_WEIGHT_BOLD);
	gtk_widget_modify_font(preview->name, font);
	pango_font_description_free(font);
	gtk_box_pack_start(GTK_BOX(vbox), preview->name, FALSE, TRUE, 0);
	/* mime */
	widget = gtk_hbox_new(FALSE, 4);
	preview->open = gtk_button_new_from_stock(GTK_STOCK_OPEN);
	gtk_size_group_add_widget(group, preview->open);
	g_signal_connect_swapped(preview->open, "clicked", G_CALLBACK(
				_preview_on_open), preview);
	gtk_box_pack_start(GTK_BOX(widget), preview->open, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	widget = gtk_hbox_new(FALSE, 4);
	preview->edit = gtk_button_new_from_stock(GTK_STOCK_EDIT);
	gtk_size_group_add_widget(group, preview->edit);
	g_signal_connect_swapped(preview->edit, "clicked", G_CALLBACK(
				_preview_on_edit), preview);
	gtk_box_pack_start(GTK_BOX(widget), preview->edit, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	/* image */
	preview->view_image = gtk_image_new();
	gtk_widget_set_no_show_all(preview->view_image, TRUE);
	gtk_box_pack_start(GTK_BOX(vbox), preview->view_image, FALSE, TRUE, 0);
	/* text */
	preview->view_text = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(preview->view_text),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_no_show_all(preview->view_text, TRUE);
	font = pango_font_description_new();
	pango_font_description_set_family(font, "monospace");
	widget = gtk_text_view_new();
	preview->view_text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(
				widget));
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(widget), FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(widget), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(widget), GTK_WRAP_WORD_CHAR);
	gtk_widget_modify_font(widget, font);
	gtk_widget_show(widget);
	pango_font_description_free(font);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(
				preview->view_text), widget);
	gtk_box_pack_start(GTK_BOX(vbox), preview->view_text, TRUE, TRUE, 0);
	gtk_widget_show_all(vbox);
	return preview;
}


/* preview_destroy */
static void _preview_destroy(Preview * preview)
{
	if(preview->source != 0)
		g_source_remove(preview->source);
	free(preview->path);
	object_delete(preview);
}


/* preview_get_widget */
static GtkWidget * _preview_get_widget(Preview * preview)
{
	return preview->widget;
}


/* preview_refresh */
static void _refresh_mime(Preview * preview, Mime * mime, char const * type);
static int _refresh_name(Preview * preview, char const * path);
static void _refresh_reset(Preview * preview);

static void _preview_refresh(Preview * preview, GList * selection)
{
	char * path = (selection != NULL) ? selection->data : NULL;
	Mime * mime = preview->helper->get_mime(preview->helper->browser);
	char const image[] = "image/";
	char const text[] = "text/";
	char const * types[] = { "application/x-perl",
		"application/x-shellscript",
		"application/xml",
		"application/xslt+xml" };
	char const * type;
	size_t i;

	_refresh_reset(preview);
	if(path == NULL)
		return;
	if(_refresh_name(preview, path) != 0)
		return;
	if((type = mime_type(mime, path)) == NULL)
		return;
	_refresh_mime(preview, mime, type);
	if(strncmp(type, image, sizeof(image) - 1) == 0)
		preview->source = g_idle_add(_preview_on_idle_image, preview);
	else if(strncmp(type, text, sizeof(text) - 1) == 0)
		preview->source = g_idle_add(_preview_on_idle_text, preview);
	else
		for(i = 0; i < sizeof(types) / sizeof(*types); i++)
			if(strcmp(types[i], type) == 0)
			{
				preview->source = g_idle_add(
						_preview_on_idle_text, preview);
				break;
			}
}

static void _refresh_mime(Preview * preview, Mime * mime, char const * type)
{
	if(mime_get_handler(mime, type, "open") != NULL)
		gtk_widget_show(preview->open);
	if(mime_get_handler(mime, type, "edit") != NULL)
		gtk_widget_show(preview->edit);
}

static int _refresh_name(Preview * preview, char const * path)
{
	BrowserPluginHelper * helper = preview->helper;
	gchar * p;

	free(preview->path);
	if((preview->path = strdup(path)) == NULL)
		return -helper->error(helper->browser, strerror(errno), 1);
	p = g_filename_display_basename(path);
	gtk_label_set_text(GTK_LABEL(preview->name), p);
	g_free(p);
	return 0;
}

static void _refresh_reset(Preview * preview)
{
	if(preview->source != 0)
		g_source_remove(preview->source);
	preview->source = 0;
	gtk_widget_hide(preview->open);
	gtk_widget_hide(preview->edit);
	gtk_widget_hide(preview->view_image);
	gtk_widget_hide(preview->view_text);
}


/* callbacks */
/* preview_on_edit */
static void _preview_on_edit(gpointer data)
{
	Preview * preview = data;
	Mime * mime = preview->helper->get_mime(preview->helper->browser);

	if(preview->path != NULL)
		mime_action(mime, "edit", preview->path);
}


/* preview_on_idle_image */
static gboolean _preview_on_idle_image(gpointer data)
{
	Preview * preview = data;
	BrowserPluginHelper * helper = preview->helper;
	GdkPixbuf * pixbuf;
	GError * error = NULL;

	preview->source = 0;
#if GTK_CHECK_VERSION(2, 6, 0)
	if((pixbuf = gdk_pixbuf_new_from_file_at_scale(preview->path, 96, 96,
					TRUE, &error)) == NULL)
#else
	if((pixbuf = gdk_pixbuf_new_from_file_at_size(preview->path, 96, 96,
					&error)) == NULL)
#endif
	{
		helper->error(helper->browser, error->message, 1);
		g_error_free(error);
		return FALSE;
	}
	gtk_image_set_from_pixbuf(GTK_IMAGE(preview->view_image), pixbuf);
	g_object_unref(pixbuf);
	gtk_widget_show(preview->view_image);
	return FALSE;
}


/* preview_on_idle_text */
static gboolean _preview_on_idle_text(gpointer data)
{
	Preview * preview = data;
	BrowserPluginHelper * helper = preview->helper;
	int fd;
	char buf[256];
	ssize_t s;

	preview->source = 0;
	gtk_text_buffer_set_text(preview->view_text_buffer, "", 0);
	if((fd = open(preview->path, O_RDONLY)) < 0)
	{
		helper->error(helper->browser, strerror(errno), 1);
		return FALSE;
	}
	/* FIXME use a GIOChannel instead */
	if((s = read(fd, buf, sizeof(buf))) > 0)
	{
		if(s == sizeof(buf))
		{
			buf[sizeof(buf) - 3] = '.';
			buf[sizeof(buf) - 2] = '.';
			buf[sizeof(buf) - 1] = '.';
		}
		gtk_text_buffer_set_text(preview->view_text_buffer, buf, s);
	}
	close(fd);
	gtk_widget_show(preview->view_text);
	return FALSE;
}


/* preview_on_open */
static void _preview_on_open(gpointer data)
{
	Preview * preview = data;
	Mime * mime = preview->helper->get_mime(preview->helper->browser);

	if(preview->path != NULL)
		mime_action(mime, "open", preview->path);
}
