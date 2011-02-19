/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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
/* TODO:
 * - add a file count and disk usage tab for directories */



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
	char * filename;
	uid_t uid;
	gid_t gid;

	/* widgets */
	GtkWidget * window;
	GtkWidget * combo;
	GtkWidget * owner;
	GtkWidget * size;
	GtkWidget * atime;
	GtkWidget * mtime;
	GtkWidget * ctime;
	GtkWidget * mode[9];
} Properties;


/* variables */
static unsigned int _properties_cnt = 0; /* XXX set as static in _properties */

/* functions */
static int _properties_error(Properties * properties, char const * message,
		int ret);
static int _properties_do(Mime * mime, GtkIconTheme * theme,
		char const * filename);
static int _properties_refresh(Properties * properties);

/* callbacks */
static void _properties_on_apply(gpointer data);
static void _properties_on_close(gpointer data);
static gboolean _properties_on_closex(gpointer data);
static void _properties_on_refresh(gpointer data);

static int _properties(Mime * mime, int filec, char * const filev[])
{
	int ret = 0;
	GtkIconTheme * theme = NULL;
	int i;

	if(mime != NULL)
		theme = gtk_icon_theme_get_default();
	for(i = 0; i < filec; i++)
	{
		_properties_cnt++;
		/* FIXME if relative path get the full path */
		ret |= _properties_do(mime, theme, filev[i]);
	}
	return ret;
}


/* _properties_error */
static void _error_response(GtkWidget * widget, gint arg, gpointer data);

static int _properties_error(Properties * properties, char const * message,
		int ret)
{
	GtkWidget * dialog;
	char const * error;

	error = strerror(errno);
	dialog = gtk_message_dialog_new((properties != NULL)
			? GTK_WINDOW(properties->window) : NULL, 0,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", _("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s: %s", message, error);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
	if(properties != NULL)
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(
					properties->window));
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				_error_response), (ret != 0)
			? &_properties_cnt : NULL);
	gtk_widget_show(dialog);
	return ret;
}

static void _error_response(GtkWidget * widget, gint arg, gpointer data)
{
	unsigned int * cnt = data;

	if(cnt == NULL)
		gtk_widget_destroy(widget);
	else if(--(*cnt) == 0)
		gtk_main_quit();
	else
		gtk_widget_destroy(widget);
}


/* _properties_do */
static char * _do_group(char * buf, size_t buf_cnt, gid_t gid);
static GtkWidget * _do_groups(Properties * properties);

static int _properties_do(Mime * mime, GtkIconTheme * theme,
		char const * filename)
{
	Properties * properties;
	struct stat st;
	char * gfilename;
	char const * type = NULL;
	struct stat dirst;
	GdkPixbuf * pixbuf = NULL;
	GtkWidget * image = NULL;
	char buf[256];
	GtkWidget * vbox;
	GtkWidget * hbox;
	size_t i;
	GtkWidget * bbox;
	GtkWidget * table;
	GtkWidget * widget;
	PangoFontDescription * bold;
	char * p;

	if(lstat(filename, &st) != 0)
		return -_properties_error(NULL, filename, 1);
	if((properties = malloc(sizeof(*properties))) == NULL)
		return -_properties_error(NULL, "malloc", 1);
	properties->filename = strdup(filename);
	properties->uid = st.st_uid;
	properties->gid = st.st_gid;
	properties->combo = NULL;
	if(properties->filename == NULL)
	{
		/* XXX warn the user */
		free(properties);
		return -1;
	}
	gfilename = g_filename_display_name(filename);
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
		if((p = strdup(filename)) != NULL
				&& lstat(dirname(p), &dirst) == 0
				&& st.st_dev != dirst.st_dev)
			type = "inode/mountpoint";
		free(p);
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
	if(image == NULL && theme != NULL && (pixbuf = gtk_icon_theme_load_icon(
					theme, "gnome-fs-regular", 48, 0, NULL))
			!= NULL)
		image = gtk_image_new_from_pixbuf(pixbuf);
	if(image == NULL)
		image = gtk_image_new_from_stock(GTK_STOCK_FILE,
				GTK_ICON_SIZE_DIALOG);
	if(image == NULL)
		image = gtk_image_new_from_stock(GTK_STOCK_MISSING_IMAGE,
				GTK_ICON_SIZE_DIALOG);
	properties->window = gtk_dialog_new();
	p = g_filename_display_basename(filename);
	snprintf(buf, sizeof(buf), "%s%s", _("Properties of "), p);
	g_free(p);
	gtk_window_set_title(GTK_WINDOW(properties->window), buf);
	gtk_window_set_resizable(GTK_WINDOW(properties->window), FALSE);
	g_signal_connect_swapped(G_OBJECT(properties->window), "delete-event",
			G_CALLBACK(_properties_on_closex), properties);
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(properties->window));
#else
	vbox = GTK_DIALOG(properties->window)->vbox;
#endif
	table = gtk_table_new(12, 2, FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(table), 4);
	gtk_table_set_row_spacings(GTK_TABLE(table), 4);
	gtk_table_set_col_spacings(GTK_TABLE(table), 4);
	gtk_table_attach_defaults(GTK_TABLE(table), image, 0, 1, 0, 2);
	widget = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(widget), gfilename);
	gtk_editable_set_editable(GTK_EDITABLE(widget), FALSE);
	bold = pango_font_description_new();
	pango_font_description_set_weight(bold, PANGO_WEIGHT_BOLD);
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, 0, 1);
	widget = gtk_label_new(type);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 1, 2, 1, 2);
	widget = gtk_label_new(_("Size:"));
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 2, 3);
	properties->size = gtk_label_new("");
	gtk_table_attach_defaults(GTK_TABLE(table), properties->size, 1, 2, 2,
			3);
	widget = gtk_label_new(_("Owner:")); /* owner name */
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 3, 4);
	properties->owner = gtk_label_new("");
	gtk_table_attach_defaults(GTK_TABLE(table), properties->owner, 1, 2, 3,
			4);
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
	properties->atime = gtk_label_new("");
	gtk_table_attach_defaults(GTK_TABLE(table), properties->atime, 1, 2, 5,
			6);
	widget = gtk_label_new(_("Modified:")); /* last modification */
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 6, 7);
	properties->mtime = gtk_label_new("");
	gtk_table_attach_defaults(GTK_TABLE(table), properties->mtime, 1, 2, 6,
			7);
	widget = gtk_label_new(_("Changed:")); /* last change */
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 7, 8);
	properties->ctime = gtk_label_new("");
	gtk_table_attach_defaults(GTK_TABLE(table), properties->ctime, 1, 2, 7,
			8);
	widget = gtk_label_new(_("Permissions:")); /* permissions */
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 8, 9);
	hbox = gtk_hbox_new(TRUE, 4);
	for(i = 0; i < sizeof(properties->mode) / sizeof(*properties->mode);
			i++)
	{
		if(hbox == NULL)
			hbox = gtk_hbox_new(TRUE, 4);
		properties->mode[i] = gtk_check_button_new_with_label("");
		gtk_box_pack_end(GTK_BOX(hbox), properties->mode[i], TRUE,
				TRUE, 0);
		if((i % 3) != 2)
			continue;
		gtk_table_attach_defaults(GTK_TABLE(table), hbox, 1, 2,
				11 - (i / 3), 12 - (i / 3));
		hbox = NULL;
	}
	widget = gtk_label_new(_("Owner:"));
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 9, 10);
	widget = gtk_label_new(_("Group:"));
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 10, 11);
	widget = gtk_label_new(_("Others:"));
	gtk_widget_modify_font(widget, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), widget, 0, 1, 11, 12);
	gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 0);
	/* button box */
#if GTK_CHECK_VERSION(2, 14, 0)
	bbox = gtk_dialog_get_action_area(GTK_DIALOG(properties->window));
#else
	bbox = GTK_DIALOG(properties->window)->action_area;
#endif
	if(properties != NULL)
	{
		widget = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
		g_signal_connect_swapped(G_OBJECT(widget), "clicked",
				G_CALLBACK(_properties_on_refresh), properties);
		gtk_container_add(GTK_CONTAINER(bbox), widget);
		widget = gtk_button_new_from_stock(GTK_STOCK_APPLY);
		g_signal_connect_swapped(G_OBJECT(widget), "clicked",
				G_CALLBACK(_properties_on_apply), properties);
		gtk_container_add(GTK_CONTAINER(bbox), widget);
	}
	widget = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_properties_on_close), properties);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	pango_font_description_free(bold);
	g_free(gfilename);
	if(_properties_refresh(properties) != 0)
	{
		gtk_widget_destroy(properties->window);
		free(properties->filename);
		free(properties);
		return 1;
	}
	gtk_widget_show_all(properties->window);
	return 0;
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
		_properties_error(properties, properties->filename, 0);
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
		_properties_error(properties, properties->filename, 0);
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


/* properties_refresh */
static void _refresh_mode(GtkWidget ** widget, mode_t mode, gboolean sensitive);
static void _refresh_owner(Properties * properties, uid_t uid);
static void _refresh_size(Properties * properties, size_t size);
static void _refresh_time(GtkWidget * widget, time_t time);

static int _properties_refresh(Properties * properties)
{
	struct stat st;
	gboolean writable;

	if(lstat(properties->filename, &st) != 0)
		return _properties_error(NULL, properties->filename, 0) + 1;
	properties->uid = st.st_uid;
	properties->gid = st.st_gid;
	writable = (access(properties->filename, W_OK) == 0) ? TRUE : FALSE;
	_refresh_mode(&properties->mode[6], (st.st_mode & 0700) >> 6, writable);
	_refresh_mode(&properties->mode[3], (st.st_mode & 0070) >> 3, writable);
	_refresh_mode(&properties->mode[0], st.st_mode & 0007, writable);
	_refresh_owner(properties, st.st_uid);
	/* FIXME also refresh the group */
	_refresh_size(properties, st.st_size);
	_refresh_time(properties->atime, st.st_atime);
	_refresh_time(properties->mtime, st.st_mtime);
	_refresh_time(properties->ctime, st.st_ctime);
	return 0;
}

static void _refresh_mode(GtkWidget ** widget, mode_t mode, gboolean sensitive)
{
	gtk_button_set_label(GTK_BUTTON(widget[2]), _("read")); /* read */
	gtk_widget_set_sensitive(widget[2], sensitive);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget[2]),
			mode & S_IROTH);
	gtk_button_set_label(GTK_BUTTON(widget[1]), _("write")); /* write */
	gtk_widget_set_sensitive(widget[1], sensitive);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget[1]),
			mode & S_IWOTH);
	gtk_button_set_label(GTK_BUTTON(widget[0]), _("execute")); /* execute */
	gtk_widget_set_sensitive(widget[0], sensitive);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget[0]),
			mode & S_IXOTH);
}

static void _refresh_owner(Properties * properties, uid_t uid)
{
	char buf[256];
	char const * p = buf;
	struct passwd * pw;

	if((pw = getpwuid(uid)) != NULL)
		p = pw->pw_name;
	else
		snprintf(buf, sizeof(buf), "%lu", (unsigned long)uid);
	gtk_label_set_text(GTK_LABEL(properties->owner), p);
}

static void _refresh_size(Properties * properties, size_t size)
{
	char buf[256];
	double sz = size;
	char * unit = _("bytes");
	char const * format = "%.1f %s";

	if(sz < 1024)
		format = "%.0f %s";
	else if((sz /= 1024) < 1024)
		unit = _("kB");
	else if((sz /= 1024) < 1024)
		unit = _("MB");
	else if((sz /= 1024) < 1024)
		unit = _("GB");
	else
	{
		sz /= 1024;
		unit = _("TB");
	}
	snprintf(buf, sizeof(buf), format, sz, unit);
	gtk_label_set_text(GTK_LABEL(properties->size), buf);
}

static void _refresh_time(GtkWidget * widget, time_t t)
{
	char buf[256];
	time_t sixmonths;
	struct tm tm;

	sixmonths = time(NULL) - 15552000;
	localtime_r(&t, &tm);
	if(t < sixmonths)
		strftime(buf, sizeof(buf), "%b %d %Y", &tm);
	else
		strftime(buf, sizeof(buf), "%b %d %H:%M", &tm);
	gtk_label_set_text(GTK_LABEL(widget), buf);
}


/* callbacks */
static void _properties_on_apply(gpointer data)
{
	Properties * properties = data;
	char * p;
	struct group * gr;
	gid_t gid = properties->gid;
	size_t i;
	mode_t mode = 0;

	p = gtk_combo_box_get_active_text(GTK_COMBO_BOX(properties->combo));
	if((gr = getgrnam(p)) == NULL)
		_properties_error(properties, p, 0);
	else
		gid = gr->gr_gid;
	for(i = 0; i < 9; i++)
		mode |= gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
					properties->mode[i])) << i;
	if(chown(properties->filename, properties->uid, gid) != 0
			|| chmod(properties->filename, mode) != 0)
		_properties_error(properties, properties->filename, 0);
}

static void _properties_on_close(gpointer data)
{
	Properties * properties = data;

	if(--_properties_cnt == 0)
		gtk_main_quit();
	else
		gtk_widget_destroy(properties->window);
}

static gboolean _properties_on_closex(gpointer data)
{
	_properties_on_close(data);
	return FALSE;
}

static void _properties_on_refresh(gpointer data)
{
	Properties * properties = data;

	_properties_refresh(properties);
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
	Mime * mime;

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
	mime = mime_new();
	ret = _properties(mime, argc - optind, &argv[optind]);
	gtk_main();
	if(mime != NULL)
		mime_delete(mime);
	return (ret == 0) ? 0 : 2;
}
