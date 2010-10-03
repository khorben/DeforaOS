/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include "mime.h"
#include "../config.h"
#define _(string) gettext(string)


/* constants */
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef DATADIR
# define DATADIR	PREFIX "/share"
#endif
#ifndef LOCALEDIR
# define LOCALEDIR	DATADIR "/locale"
#endif


/* properties */
/* types */
typedef struct _Properties
{
	char const * filename;
	uid_t uid;
	gid_t gid;

	/* widgets */
	GtkWidget * window;
	GtkWidget * combo;
	GtkWidget * check[9];
} Properties;


/* variables */
static unsigned int _properties_cnt = 0; /* XXX set as static in _properties */

/* functions */
static int _properties_error(GtkWidget * window, char const * message, int ret);
static int _properties_do(Mime * mime, GtkIconTheme * theme,
		char const * filename);

/* callbacks */
static gboolean _properties_on_closex(GtkWidget * widget);
static void _properties_on_close(GtkWidget * widget);
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
			"%s: %s", message,
#if GTK_CHECK_VERSION(2, 6, 0)
			_("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s: %s", message,
#endif
			strerror(errno));
	if(window != NULL)
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(
					window));
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
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
static char * _do_time(char * buf, size_t buf_cnt, time_t date);
static GtkWidget * _do_groups(Properties * properties);
static GtkWidget * _do_mode(GtkWidget ** widget, mode_t mode);

static int _properties_do(Mime * mime, GtkIconTheme * theme,
		char const * filename)
{
	struct stat st;
	Properties * properties = NULL;
	char const * gfilename;
	char * p;
	char const * type = NULL;
	struct stat dirst;
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
		properties->combo = NULL;
	}
	if((gfilename = g_filename_to_utf8(filename, -1, NULL, NULL, NULL))
			== NULL)
		gfilename = filename;
	p = strdup(gfilename);
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
		if(p != NULL && lstat(dirname(p), &dirst) == 0
				&& st.st_dev != dirst.st_dev)
			type = "inode/mountpoint";
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
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	if(properties != NULL)
		properties->window = window;
	snprintf(buf, sizeof(buf), "%s%s", _("Properties of "), basename(p));
	gtk_window_set_title(GTK_WINDOW(window), buf);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				_properties_on_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 0);
	table = gtk_table_new(12, 2, FALSE);
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
	widget = gtk_label_new(_("Size:")); /* XXX justification doesn't work */
	gtk_widget_modify_font(widget, bold);
	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 2, 3);
	widget = gtk_label_new(_do_size(buf, sizeof(buf), st.st_size));
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, 2, 3);
	widget = gtk_label_new(_("Owner:")); /* owner name */
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 3, 4);
	widget = gtk_label_new(_do_owner(buf, sizeof(buf), st.st_uid));
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, 3, 4);
	widget = gtk_label_new(_("Group:")); /* group name */
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 4, 5);
	if(properties == NULL)
		widget = gtk_label_new(_do_group(buf, sizeof(buf), st.st_gid));
	else
		widget = _do_groups(properties);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, 4, 5);
	widget = gtk_label_new(_("Accessed:")); /* last access */
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 5, 6);
	widget = gtk_label_new(_do_time(buf, sizeof(buf), st.st_mtime));
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, 5, 6);
	widget = gtk_label_new(_("Modified:")); /* last modification */
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 6, 7);
	widget = gtk_label_new(_do_time(buf, sizeof(buf), st.st_mtime));
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, 6, 7);
	widget = gtk_label_new(_("Changed:")); /* last change */
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 7, 8);
	widget = gtk_label_new(_do_time(buf, sizeof(buf), st.st_mtime));
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, 7, 8);
	widget = gtk_label_new(_("Permissions:")); /* permissions */
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 8, 9);
	widget = gtk_label_new(_("Owner:")); /* owner permissions */
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 9, 10);
	widget = _do_mode(properties ? &properties->check[6] : NULL,
			(st.st_mode & 0700) >> 6);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, 9, 10);
	widget = gtk_label_new(_("Group:")); /* group permissions */
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 10, 11);
	widget = _do_mode(properties ? &properties->check[3] : NULL,
			(st.st_mode & 0070) >> 3);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, 10, 11);
	widget = gtk_label_new(_("Others:")); /* others permissions */
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 11, 12);
	widget = _do_mode(properties ? &properties->check[0] : NULL,
			st.st_mode & 0007);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, 11, 12);
	gtk_box_pack_start(GTK_BOX(hbox), table, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	hbox = gtk_hbox_new(FALSE, 4); /* separator */
	widget = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, 0); /* close button */
	widget = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_properties_on_close), NULL);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	if(properties != NULL)
	{
		widget = gtk_button_new_from_stock(GTK_STOCK_APPLY);
		g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
					_properties_on_apply), properties);
		gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	}
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
	pango_font_description_free(bold);
	free(p);
	return 0;
}

static char * _do_size(char * buf, size_t buf_cnt, size_t size)
{
	double sz = size;
	char * unit;

	if(sz < 1024)
	{
		snprintf(buf, buf_cnt, "%.0f%s", sz, _(" bytes"));
		return buf;
	}
	else if((sz /= 1024) < 1024)
		unit = _("KB");
	else if((sz /= 1024) < 1024)
		unit = _("MB");
	else if((sz /= 1024) < 1024)
		unit = _("GB");
	else
	{
		sz /= 1024;
		unit = _("TB");
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

static char * _do_time(char * buf, size_t buf_cnt, time_t date)
{
	static time_t sixmonths = -1;
	struct tm tm;

	if(sixmonths == -1)
		sixmonths = time(NULL) - 15552000;
	localtime_r(&date, &tm);
	if(date < sixmonths)
		strftime(buf, buf_cnt, "%b %d %Y", &tm);
	else
		strftime(buf, buf_cnt, "%b %d %H:%M", &tm);
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

static GtkWidget * _do_mode(GtkWidget ** widget, mode_t mode)
{
	GtkWidget * hbox;
	GtkWidget * w[3];
	gboolean sensitive = FALSE;

	if(widget == NULL)
		widget = w;
	else
		sensitive = TRUE;
	hbox = gtk_hbox_new(TRUE, 0);
	widget[2] = gtk_check_button_new_with_label(_("read")); /* read */
	gtk_widget_set_sensitive(widget[2], sensitive);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget[2]),
			mode & S_IROTH);
	gtk_box_pack_start(GTK_BOX(hbox), widget[2], TRUE, TRUE, 4);
	widget[1] = gtk_check_button_new_with_label(_("write")); /* write */
	gtk_widget_set_sensitive(widget[1], sensitive);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget[1]),
			mode & S_IWOTH);
	gtk_box_pack_start(GTK_BOX(hbox), widget[1], TRUE, TRUE, 4);
	widget[0] = gtk_check_button_new_with_label(_("execute")); /* execute */
	gtk_widget_set_sensitive(widget[0], sensitive);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget[0]),
			mode & S_IXOTH);
	gtk_box_pack_start(GTK_BOX(hbox), widget[0], TRUE, TRUE, 4);
	return hbox;
}


/* callbacks */
static gboolean _properties_on_closex(GtkWidget * widget)
{
	_properties_on_close(widget);
	return FALSE;
}

static void _properties_on_close(GtkWidget * widget)
{
	if(--_properties_cnt == 0)
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
	size_t i;
	mode_t mode = 0;

	p = gtk_combo_box_get_active_text(GTK_COMBO_BOX(properties->combo));
	if((gr = getgrnam(p)) == NULL)
		_properties_error(properties->window, p, 0);
	else
		gid = gr->gr_gid;
	for(i = 0; i < 9; i++)
		mode |= gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
					properties->check[i])) << i;
	if(chown(properties->filename, properties->uid, gid) != 0
			|| chmod(properties->filename, mode) != 0)
		_properties_error(gtk_widget_get_toplevel(widget),
				properties->filename, 0);
}


/* usage */
static int _usage(void)
{
	fputs(_("Usage: properties file...\n"), stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int ret;
	int o;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
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
