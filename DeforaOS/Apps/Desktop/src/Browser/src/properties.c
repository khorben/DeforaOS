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
#include <Desktop.h>
#include "../config.h"
#define _(string) gettext(string)
#define N_(string) (string)


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
	Mime * mime;
	char * filename;
	uid_t uid;
	gid_t gid;

	/* widgets */
	GtkIconTheme * theme;
	GtkWidget * window;
	GtkWidget * view;
	GtkWidget * name;
	GtkWidget * type;
	GtkWidget * image;
	GtkWidget * owner;
	GtkWidget * group;
	GtkWidget * size;
	GtkWidget * atime;
	GtkWidget * mtime;
	GtkWidget * ctime;
	GtkWidget * mode[9];
} Properties;


/* variables */
static unsigned int _properties_cnt = 0; /* XXX set as static in _properties */

/* functions */
#ifdef WITH_MAIN
static int _properties(Mime * mime, int filec, char * const filev[]);
#endif

static Properties * _properties_new(char const * filename, Mime * mime);
static void _properties_delete(Properties * properties);

#ifndef WITH_MAIN
static GtkWidget * _properties_get_view(Properties * properties);
#endif
static int _properties_set_filename(Properties * properties,
		char const * filename);

static int _properties_error(Properties * properties, char const * message,
		int ret);
static int _properties_refresh(Properties * properties);
#ifdef WITH_MAIN
static void _properties_show_window(Properties * properties);

/* callbacks */
static void _properties_on_apply(gpointer data);
static void _properties_on_close(gpointer data);
static gboolean _properties_on_closex(gpointer data);
static void _properties_on_refresh(gpointer data);
#endif


#ifdef WITH_MAIN
/* properties */
static int _properties(Mime * mime, int filec, char * const filev[])
{
	int ret = 0;
	int i;
	Properties * properties;

	for(i = 0; i < filec; i++)
	{
		/* FIXME if relative path get the full path */
		if((properties = _properties_new(filev[i], mime))
				== NULL)
		{
			ret |= 1;
			continue;
		}
		_properties_show_window(properties);
		_properties_cnt++;
	}
	return ret;
}
#endif


/* properties_new */
static Properties * _properties_new(char const * filename, Mime * mime)
{
	Properties * properties;
	GtkWidget * vbox;
	GtkWidget * table;
	GtkWidget * hbox;
	GtkWidget * widget;
	PangoFontDescription * bold;
	size_t i;

	if((properties = malloc(sizeof(*properties))) == NULL)
	{
		_properties_error(NULL, "malloc", 1);
		return NULL;
	}
	properties->mime = mime;
	properties->filename = NULL;
	properties->theme = gtk_icon_theme_get_default();
	properties->window = NULL;
	properties->group = NULL;
	properties->view = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(properties->view),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(
				properties->view), GTK_SHADOW_NONE);
	vbox = gtk_vbox_new(FALSE, 4);
	table = gtk_table_new(12, 2, FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(table), 4);
	gtk_table_set_row_spacings(GTK_TABLE(table), 4);
	gtk_table_set_col_spacings(GTK_TABLE(table), 4);
	properties->image = gtk_image_new();
	gtk_table_attach_defaults(GTK_TABLE(table), properties->image, 0, 1, 0,
			2);
	properties->name = gtk_entry_new();
	gtk_editable_set_editable(GTK_EDITABLE(properties->name), FALSE);
	bold = pango_font_description_new();
	pango_font_description_set_weight(bold, PANGO_WEIGHT_BOLD);
	gtk_widget_modify_font(properties->name, bold);
	gtk_table_attach_defaults(GTK_TABLE(table), properties->name, 1, 2, 0,
			1);
	properties->type = gtk_label_new(NULL);
	gtk_table_attach_defaults(GTK_TABLE(table), properties->type, 1, 2, 1,
			2);
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
	properties->group = gtk_combo_box_new_text();
	gtk_table_attach_defaults(GTK_TABLE(table), properties->group, 1, 2, 4,
			5);
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
	hbox = gtk_hbox_new(TRUE, 4);
	widget = gtk_label_new(_("Read:"));
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_widget_modify_font(widget, bold);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	widget = gtk_label_new(_("Write:"));
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_widget_modify_font(widget, bold);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	widget = gtk_label_new(_("Execute:"));
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_widget_modify_font(widget, bold);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	gtk_table_attach_defaults(GTK_TABLE(table), hbox, 1, 2, 8, 9);
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
	pango_font_description_free(bold);
	if(filename != NULL)
		_properties_set_filename(properties, filename);
	gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, TRUE, 0);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(
				properties->view), vbox);
	gtk_widget_set_size_request(properties->view, 200, 320);
	gtk_widget_show_all(properties->view);
	return properties;
}


/* properties_delete */
static void _properties_delete(Properties * properties)
{
	if(properties->window != NULL)
		gtk_widget_destroy(properties->window);
	free(properties->filename);
	free(properties);
	_properties_cnt--;
}


#ifndef WITH_MAIN
/* properties_get_view */
static GtkWidget * _properties_get_view(Properties * properties)
{
	return properties->view;
}
#endif


/* properties_set_filename */
static int _properties_set_filename(Properties * properties,
		char const * filename)
{
	char * p;

	if((p = strdup(filename)) == NULL)
		return -_properties_error(NULL, filename, 1);
	free(properties->filename);
	properties->filename = p;
	return _properties_refresh(properties);
}


/* _properties_error */
static void _error_response(GtkWidget * widget, gint arg, gpointer data);

static int _properties_error(Properties * properties, char const * message,
		int ret)
{
	GtkWidget * dialog;
	char const * error;

	error = strerror(errno);
	dialog = gtk_message_dialog_new((properties != NULL
				&& properties->window != NULL)
			? GTK_WINDOW(properties->window) : NULL, 0,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", _("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s: %s", message, error);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
	if(properties != NULL && properties->window != NULL)
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


/* properties_refresh */
static void _refresh_name(GtkWidget * widget, char const * filename);
static void _refresh_type(Properties * properties, struct stat * st);
static void _refresh_mode(GtkWidget ** widget, mode_t mode, gboolean sensitive);
static void _refresh_owner(Properties * properties, uid_t uid);
static int _refresh_group(Properties * properties, gid_t gid);
static void _refresh_size(Properties * properties, size_t size);
static void _refresh_time(GtkWidget * widget, time_t time);

static int _properties_refresh(Properties * properties)
{
	struct stat st;
	gboolean writable;

	if(lstat(properties->filename, &st) != 0)
		return _properties_error(NULL, properties->filename, 0) + 1;
	_refresh_name(properties->name, properties->filename);
	_refresh_type(properties, &st);
	properties->uid = st.st_uid;
	properties->gid = st.st_gid;
	writable = (access(properties->filename, W_OK) == 0) ? TRUE : FALSE;
	_refresh_mode(&properties->mode[6], (st.st_mode & 0700) >> 6, writable);
	_refresh_mode(&properties->mode[3], (st.st_mode & 0070) >> 3, writable);
	_refresh_mode(&properties->mode[0], st.st_mode & 0007, writable);
	_refresh_owner(properties, st.st_uid);
	_refresh_group(properties, st.st_gid);
	_refresh_size(properties, st.st_size);
	_refresh_time(properties->atime, st.st_atime);
	_refresh_time(properties->mtime, st.st_mtime);
	_refresh_time(properties->ctime, st.st_ctime);
	return 0;
}

static void _refresh_name(GtkWidget * widget, char const * filename)
{
	gchar * gfilename;

#ifdef WITH_MAIN
	gfilename = g_filename_display_name(filename);
#else
	gfilename = g_filename_display_basename(filename);
#endif
	gtk_entry_set_text(GTK_ENTRY(widget), gfilename);
	g_free(gfilename);
}

static void _refresh_type(Properties * properties, struct stat * st)
{
	char const * type = NULL;
	GdkPixbuf * pixbuf = NULL;
	GtkWidget * image = NULL;
	char * p;
	struct stat dirst;

	if(S_ISDIR(st->st_mode))
	{
		if((pixbuf = gtk_icon_theme_load_icon(properties->theme,
						"gnome-fs-directory", 48, 0,
						NULL)) != NULL)
			image = gtk_image_new_from_pixbuf(pixbuf);
		if(image == NULL)
			image = gtk_image_new_from_stock(GTK_STOCK_DIRECTORY,
					GTK_ICON_SIZE_DIALOG);
		type = "inode/directory";
		if((p = strdup(properties->filename)) != NULL
				&& lstat(dirname(p), &dirst) == 0
				&& st->st_dev != dirst.st_dev)
			type = "inode/mountpoint";
		free(p);
	}
	else if(S_ISBLK(st->st_mode))
		type = "inode/blockdevice";
	else if(S_ISBLK(st->st_mode))
		type = "inode/chardevice";
	else if(S_ISFIFO(st->st_mode))
		type = "inode/fifo";
	else if(S_ISLNK(st->st_mode))
		type = "inode/symlink";
#ifdef S_ISSOCK
	else if(S_ISSOCK(st->st_mode))
		type = "inode/socket";
#endif
	else if(properties->mime != NULL)
	{
		type = mime_type(properties->mime, properties->filename);
		if(type != NULL)
		{
			mime_icons(properties->mime, type, 48, &pixbuf, -1);
			if(pixbuf != NULL)
				image = gtk_image_new_from_pixbuf(pixbuf);
		}
	}
	else
		type = _("Unknown type");
	gtk_label_set_text(GTK_LABEL(properties->type), type);
	if(image == NULL && (pixbuf = gtk_icon_theme_load_icon(
					properties->theme, "gnome-fs-regular",
					48, 0, NULL)) != NULL)
		image = gtk_image_new_from_pixbuf(pixbuf);
	if(image == NULL)
		image = gtk_image_new_from_stock(GTK_STOCK_FILE,
				GTK_ICON_SIZE_DIALOG);
	if(image == NULL)
		image = gtk_image_new_from_stock(GTK_STOCK_MISSING_IMAGE,
				GTK_ICON_SIZE_DIALOG);
	gtk_image_set_from_pixbuf(GTK_IMAGE(properties->image),
			gtk_image_get_pixbuf(GTK_IMAGE(image)));
	gtk_widget_destroy(image);
}

static void _refresh_mode(GtkWidget ** widget, mode_t mode, gboolean sensitive)
{
	gtk_widget_set_sensitive(widget[2], sensitive);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget[2]),
			mode & S_IROTH);
	gtk_widget_set_sensitive(widget[1], sensitive);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget[1]),
			mode & S_IWOTH);
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

static int _refresh_group(Properties * properties, gid_t gid)
{
	GtkWidget * combo;
	GtkListStore * store;
	int i = 0;
	int active;
	struct passwd * pw;
	struct group * gr;
	char ** p;

	combo = properties->group;
	store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(combo)));
	gtk_list_store_clear(store);
	if((gr = getgrgid(getgid())) == NULL)
		return -_properties_error(properties, properties->filename, 1);
	gtk_combo_box_insert_text(GTK_COMBO_BOX(combo), i, gr->gr_name);
	active = i++;
	if((pw = getpwuid(getuid())) == NULL)
		return -_properties_error(properties, properties->filename, 1);
	setgrent();
	for(gr = getgrent(); gr != NULL; gr = getgrent())
		for(p = gr->gr_mem; p != NULL && *p != NULL; p++)
			if(strcmp(pw->pw_name, *p) == 0)
			{
				if(gid == gr->gr_gid)
					active = i;
				gtk_combo_box_insert_text(GTK_COMBO_BOX(combo),
						i++, gr->gr_name);
			}
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo), active);
	return 0;
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


#ifdef WITH_MAIN
/* properties_show_window */
static void _properties_show_window(Properties * properties)
{
	GtkWidget * vbox;
	GtkWidget * bbox;
	GtkWidget * widget;
	gchar * p;
	char buf[256];

	if(properties->window != NULL)
	{
		gtk_window_present(GTK_WINDOW(properties->window));
		return;
	}
	properties->window = gtk_dialog_new();
	p = g_filename_display_basename(properties->filename);
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
	gtk_box_pack_start(GTK_BOX(vbox), properties->view, FALSE, TRUE, 0);
	/* button box */
#if GTK_CHECK_VERSION(2, 14, 0)
	bbox = gtk_dialog_get_action_area(GTK_DIALOG(properties->window));
#else
	bbox = GTK_DIALOG(properties->window)->action_area;
#endif
	widget = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked",
			G_CALLBACK(_properties_on_refresh), properties);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	widget = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked",
			G_CALLBACK(_properties_on_apply), properties);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	widget = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_properties_on_close), properties);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	gtk_widget_show_all(bbox);
	gtk_widget_show(properties->window);
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

	p = gtk_combo_box_get_active_text(GTK_COMBO_BOX(properties->group));
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

	_properties_delete(properties);
	if(_properties_cnt == 0)
		gtk_main_quit();
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
#endif


#ifdef WITH_MAIN
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
	mime = mime_new(NULL);
	ret = _properties(mime, argc - optind, &argv[optind]);
	gtk_main();
	if(mime != NULL)
		mime_delete(mime);
	return (ret == 0) ? 0 : 2;
}
#endif
