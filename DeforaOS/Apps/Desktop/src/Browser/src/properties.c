/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
/* Browser is not free software; you can redistribute it and/or modify it
 * under the terms of the Creative Commons Attribution-NonCommercial-ShareAlike
 * 3.0 Unported as published by the Creative Commons organization.
 *
 * Browser is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with Browser; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */
/* FIXME:
 * - add dates
 * - resolve relative paths */



#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <libgen.h>
#include <gtk/gtk.h>
#include "mime.h"


/* properties */
/* types */
typedef struct _Properties
{
	char * filename;
	uid_t uid;
	gid_t gid;
	mode_t mode;

	/* widgets */
	GtkWidget * window;
	GtkWidget * combo;
} Properties;


/* variables */
static unsigned int _properties_cnt = 0; /* XXX set as static in _properties */

/* functions */
static int _properties_error(GtkWidget * window, char const * message, int ret);
static int _properties_do(Mime * mime, GtkIconTheme * theme,
		char const * filename);

/* callbacks */
static gboolean _properties_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static void _properties_on_close(GtkWidget * widget, gpointer data);
static void _properties_on_apply(GtkWidget * widget, gpointer data);

static int _properties(int filec, char * const filev[])
{
	int ret = 0;
	Mime * mime;
	GtkIconTheme * theme = NULL;
	int i;

	if((mime = mime_new()) != NULL)
		theme = gtk_icon_theme_get_default();
	for(i = 0; i < filec; i++)
	{
		_properties_cnt++;
		/* FIXME if relative path get the full path */
		ret |= _properties_do(mime, theme, filev[i]);
	}
	if(mime != NULL)
		mime_delete(mime);
	return ret;
}


/* _properties_error */
static void _error_response(GtkDialog * dialog, gint arg, gpointer data);

static int _properties_error(GtkWidget * window, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(window != NULL ? GTK_WINDOW(window)
			: NULL, 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
			"%s: %s", message, strerror(errno));
	if(window != NULL)
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(
					window));
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				_error_response), &_properties_cnt);
	gtk_widget_show(dialog);
	return ret;
}

static void _error_response(GtkDialog * dialog, gint arg, gpointer data)
{
	unsigned int * cnt = data;

	(*cnt)--;
	if(*cnt == 0)
		gtk_main_quit();
	else
		gtk_widget_destroy(GTK_WIDGET(dialog));
}

/* _properties_do */
static char * _do_size(char * buf, size_t buf_cnt, size_t size);
static char * _do_owner(char * buf, size_t buf_cnt, uid_t uid);
static char * _do_group(char * buf, size_t buf_cnt, gid_t gid);
static GtkWidget * _do_groups(Properties * properties);
static GtkWidget * _do_mode(Properties * properties, mode_t mode);

static int _properties_do(Mime * mime, GtkIconTheme * theme,
		char const * filename)
{
	struct stat st;
	Properties * properties = NULL;
	char const * gfilename;
	char const * type = NULL;
	GdkPixbuf * pixbuf = NULL;
	GtkWidget * image = NULL;
	GtkWidget * window;
	char buf[256];
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * table;
	GtkWidget * widget;
	PangoFontDescription * bold;

	if(lstat(filename, &st) != 0)
		return _properties_error(NULL, filename, 1);
	if(access(filename, W_OK) == 0
			&& (properties = malloc(sizeof(*properties))) != NULL)
	{
		properties->filename = filename; /* no need to duplicate yet */
		properties->uid = st.st_uid;
		properties->gid = st.st_gid;
		properties->mode = st.st_mode;
		properties->combo = NULL;
	}
	if(S_ISDIR(st.st_mode))
	{
		if(theme != NULL && (pixbuf = gtk_icon_theme_load_icon(theme,
						"gnome-fs-directory", 48, 0,
						NULL)) != NULL)
			image = gtk_image_new_from_pixbuf(pixbuf);
		if(image == NULL)
			image = gtk_image_new_from_stock(GTK_STOCK_DIRECTORY,
					GTK_ICON_SIZE_DIALOG);
		type = "inode/directory";
	}
	else if(S_ISBLK(st.st_mode))
		type = "inode/blockdevice";
	else if(S_ISBLK(st.st_mode))
		type = "inode/chardevice";
	else if(S_ISFIFO(st.st_mode))
		type = "inode/fifo";
	else if(S_ISLNK(st.st_mode))
		type = "inode/symlink";
#ifdef S_ISSOCK
	else if(S_ISSOCK(st.st_mode))
		type = "inode/socket";
#endif
	else if(mime != NULL)
	{
		type = mime_type(mime, filename);
		if(theme != NULL && type != NULL)
		{
			mime_icons(mime, theme, type, 48, &pixbuf, -1);
			if(pixbuf != NULL)
				image = gtk_image_new_from_pixbuf(pixbuf);
		}
	}
	else
		type = "Unknown type";
	if(image == NULL)
	{
		if(theme != NULL && (pixbuf = gtk_icon_theme_load_icon(theme,
						"gnome-fs-regular", 48, 0,
						NULL)) != NULL)
			image = gtk_image_new_from_pixbuf(pixbuf);
		if(image == NULL)
			image = gtk_image_new_from_stock(GTK_STOCK_FILE,
					GTK_ICON_SIZE_DIALOG);
		if(image == NULL)
			image = gtk_image_new_from_stock(
					GTK_STOCK_MISSING_IMAGE,
					GTK_ICON_SIZE_DIALOG);
	}
	if((gfilename = g_filename_to_utf8(filename, -1, NULL, NULL, NULL))
			== NULL)
		gfilename = filename;
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	if(properties != NULL)
		properties->window = window;
	snprintf(buf, sizeof(buf), "%s%s", "Properties of ", basename(
				gfilename));
	gtk_window_set_title(GTK_WINDOW(window), buf);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				_properties_on_closex), &_properties_cnt);
	vbox = gtk_vbox_new(FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 0);
	table = gtk_table_new(9, 2, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table), 4);
	gtk_table_set_col_spacings(GTK_TABLE(table), 4);
	gtk_table_attach_defaults(GTK_TABLE(table), image, 0, 1, 0, 2);
	widget = gtk_label_new(gfilename);
	bold = pango_font_description_new();
	pango_font_description_set_weight(bold, PANGO_WEIGHT_BOLD);
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, 0, 1);
	widget = gtk_label_new(type);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, 1, 2);
	widget = gtk_label_new("Size:"); /* XXX justification does not work */
	gtk_widget_modify_font(widget, bold);
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 2, 3);
	widget = gtk_label_new(_do_size(buf, sizeof(buf), st.st_size));
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, 2, 3);
	widget = gtk_label_new("Owner:"); /* owner name */
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 3, 4);
	widget = gtk_label_new(_do_owner(buf, sizeof(buf), st.st_uid));
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, 3, 4);
	widget = gtk_label_new("Group:"); /* group name */
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 4, 5);
	if(properties == NULL)
		widget = gtk_label_new(_do_group(buf, sizeof(buf), st.st_gid));
	else
		widget = _do_groups(properties);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, 4, 5);
	widget = gtk_label_new("Permissions:"); /* permissions */
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 5, 6);
	widget = gtk_label_new("Owner:"); /* owner permissions */
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 6, 7);
	widget = _do_mode(properties, (st.st_mode & 0700) >> 6);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, 6, 7);
	widget = gtk_label_new("Group:"); /* group permissions */
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 7, 8);
	widget = _do_mode(properties, (st.st_mode & 0070) >> 3);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, 7, 8);
	widget = gtk_label_new("Others:"); /* others permissions */
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 8, 9);
	widget = _do_mode(properties, st.st_mode & 0007);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, 8, 9);
	gtk_box_pack_start(GTK_BOX(hbox), table, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	hbox = gtk_hbox_new(FALSE, 4); /* separator */
	widget = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, 0); /* close button */
	widget = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_properties_on_close), &_properties_cnt);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	if(properties != NULL)
	{
		widget = gtk_button_new_from_stock(GTK_STOCK_APPLY);
		g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
					_properties_on_apply), properties);
		gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	}
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
	pango_font_description_free(bold);
	return 0;
}

static char * _do_size(char * buf, size_t buf_cnt, size_t size)
{
	double sz = size;
	char * unit;

	if(sz < 1024)
	{
		snprintf(buf, buf_cnt, "%.0f%s", sz, " bytes");
		return buf;
	}
	else if((sz /= 1024) < 1024)
		unit = "KB";
	else if((sz /= 1024) < 1024)
		unit = "MB";
	else if((sz /= 1024) < 1024)
		unit = "GB";
	else
	{
		sz /= 1024;
		unit = "TB";
	}
	snprintf(buf, buf_cnt, "%.1f %s", sz, unit);
	return buf;
}

static char * _do_owner(char * buf, size_t buf_cnt, uid_t uid)
{
	struct passwd * pw;

	if((pw = getpwuid(uid)) != NULL)
		return pw->pw_name;
	snprintf(buf, buf_cnt, "%lu", (unsigned long)uid);
	return buf;
}

static char * _do_group(char * buf, size_t buf_cnt, gid_t gid)
{
	struct group * gr;

	if((gr = getgrgid(gid)) != NULL)
		return gr->gr_name;
	snprintf(buf, buf_cnt, "%lu", (unsigned long)gid);
	return buf;
}

static GtkWidget * _do_groups(Properties * properties)
{
	GtkWidget * box;
	GtkWidget * combo;
	int i = 0;
	int active;
	struct passwd * pw;
	struct group * gr;
	char ** p;

	if((gr = getgrgid(getgid())) == NULL)
	{
		_properties_error(properties->window, properties->filename, 0);
		return gtk_label_new("");
	}
	box = gtk_hbox_new(TRUE, 0);
	combo = gtk_combo_box_new_text();
	properties->combo = combo;
	gtk_combo_box_insert_text(GTK_COMBO_BOX(combo), i, gr->gr_name);
	active = i++;
	gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 0);
	if((pw = getpwuid(getuid())) == NULL)
	{
		_properties_error(properties->window, properties->filename, 0);
		return combo;
	}
	setgrent();
	for(gr = getgrent(); gr != NULL; gr = getgrent())
		for(p = gr->gr_mem; p != NULL && *p != NULL; p++)
			if(strcmp(pw->pw_name, *p) == 0)
			{
				if(properties->gid == gr->gr_gid)
					active = i;
				gtk_combo_box_insert_text(GTK_COMBO_BOX(combo),
						i++, gr->gr_name);
			}
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo), active);
	return box;
}

static GtkWidget * _do_mode(Properties * properties, mode_t mode)
{
	GtkWidget * hbox;
	GtkWidget * widget;

	hbox = gtk_hbox_new(TRUE, 0);
	widget = gtk_check_button_new_with_label("read"); /* read */
	if(properties == NULL)
		gtk_widget_set_sensitive(widget, FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), mode & S_IROTH);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	widget = gtk_check_button_new_with_label("write"); /* write */
	if(properties == NULL)
		gtk_widget_set_sensitive(widget, FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), mode & S_IWOTH);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	widget = gtk_check_button_new_with_label("execute"); /* execute */
	if(properties == NULL)
		gtk_widget_set_sensitive(widget, FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), mode & S_IXOTH);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	return hbox;
}


/* callbacks */
static gboolean _properties_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	_properties_on_close(widget, data);
	return FALSE;
}

static void _properties_on_close(GtkWidget * widget, gpointer data)
{
	unsigned int * cnt = data;

	(*cnt)--;
	if(*cnt == 0)
		gtk_main_quit();
	else
		gtk_widget_destroy(gtk_widget_get_toplevel(widget));
}

static void _properties_on_apply(GtkWidget * widget, gpointer data)
{
	Properties * properties = data;
	char * p;
	struct group * gr;
	gid_t gid = properties->gid;

	p = gtk_combo_box_get_active_text(GTK_COMBO_BOX(properties->combo));
	if((gr = getgrnam(p)) == NULL)
		_properties_error(properties->window, p, 0);
	else
		gid = gr->gr_gid;
	if(chown(properties->filename, properties->uid, gid) != 0
			|| chmod(properties->filename, properties->mode) != 0)
		_properties_error(gtk_widget_get_toplevel(widget),
				properties->filename, 0);
}


/* usage */
static int _usage(void)
{
	fputs("Usage: properties file...\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int ret;
	int o;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	ret = _properties(argc - optind, &argv[optind]) ? 0 : 2;
	gtk_main();
	return ret ? 0 : 2;
}
