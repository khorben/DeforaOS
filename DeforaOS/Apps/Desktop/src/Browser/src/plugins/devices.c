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



#include <System.h>
#include <string.h>
#include <libintl.h>
#ifdef __NetBSD__
# include <sys/types.h>
# include <sys/statvfs.h>
#endif
#include "Browser.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* Devices */
/* private */
/* types */
enum _DevicesColumn
{
	DC_PIXBUF = 0,
	DC_NAME,
	DC_MOUNTPOINT
};
#define DC_LAST DC_MOUNTPOINT
#define DC_COUNT (DC_LAST + 1)

enum _DevicesPixbuf
{
	DP_HARDDISK = 0,
	DP_CDROM,
	DP_REMOVABLE
};
#define DP_LAST DP_REMOVABLE
#define DP_COUNT (DP_LAST + 1)

typedef struct _Devices
{
	guint source;
	GtkWidget * window;
	GtkListStore * store;
	GtkWidget * view;
	GdkPixbuf * icons[DP_COUNT];
} Devices;


/* prototypes */
static GtkWidget * _devices_init(BrowserPlugin * plugin);
static void _devices_destroy(BrowserPlugin * plugin);
static void _devices_refresh(BrowserPlugin * plugin, char const * path);

/* callbacks */
static gboolean _devices_on_idle(gpointer data);
static void _devices_on_selection_changed(gpointer data);


/* public */
/* variables */
BrowserPlugin plugin =
{
	NULL,
	N_("Devices"),
	"drive-harddisk",
	_devices_init,
	_devices_destroy,
	_devices_refresh,
	NULL
};


/* private */
/* functions */
/* devices_init */
static GtkWidget * _devices_init(BrowserPlugin * plugin)
{
	Devices * devices;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;
	GtkTreeSelection * treesel;
	GtkIconTheme * icontheme;
	char const * icons[DP_COUNT] = { "drive-harddisk", "drive-cdrom",
		"drive-removable-media" };
	size_t i;
	gint width;
	gint height;

	if((devices = object_new(sizeof(*devices))) == NULL)
		return NULL;
	plugin->priv = devices;
	devices->window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(devices->window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	devices->store = gtk_list_store_new(DC_COUNT, GDK_TYPE_PIXBUF,
			G_TYPE_STRING, G_TYPE_STRING);
	devices->view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				devices->store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(devices->view), FALSE);
	renderer = gtk_cell_renderer_pixbuf_new();
	column = gtk_tree_view_column_new_with_attributes(NULL, renderer,
			"pixbuf", DC_PIXBUF, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(devices->view), column);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(NULL, renderer,
			"text", DC_NAME, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(devices->view), column);
	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(devices->view));
	g_signal_connect_swapped(G_OBJECT(treesel), "changed", G_CALLBACK(
				_devices_on_selection_changed), plugin);
	gtk_container_add(GTK_CONTAINER(devices->window), devices->view);
	icontheme = gtk_icon_theme_get_default();
	gtk_icon_size_lookup(GTK_ICON_SIZE_BUTTON, &width, &height);
	for(i = 0; i < DP_COUNT; i++)
		devices->icons[i] = gtk_icon_theme_load_icon(icontheme,
				icons[i], width, GTK_ICON_LOOKUP_USE_BUILTIN,
				NULL);
	gtk_widget_show_all(devices->window);
	devices->source = g_idle_add(_devices_on_idle, plugin);
	return devices->window;
}


/* devices_destroy */
static void _devices_destroy(BrowserPlugin * plugin)
{
	Devices * devices = plugin->priv;

	if(devices->source != 0)
		g_source_remove(devices->source);
	object_delete(devices);
}


/* devices_refresh */
static void _refresh_add(Devices * devices, char const * name,
		char const * device, char const * mountpoint,
		char const * filesystem);

static void _devices_refresh(BrowserPlugin * plugin, char const * path)
{
	Devices * devices = plugin->priv;
#ifdef __NetBSD__
	struct statvfs * mnt;
	int res;
	int i;
#endif

	gtk_list_store_clear(devices->store);
#ifdef __NetBSD__
	if((res = getmntinfo(&mnt, ST_WAIT)) <= 0)
		return;
	for(i = 0; i < res; i++)
		_refresh_add(devices, NULL, mnt[i].f_mntfromname,
				mnt[i].f_mntonname, mnt[i].f_fstypename);
#else
	_refresh_add(devices, NULL, NULL, "/", NULL);
#endif
}

static void _refresh_add(Devices * devices, char const * name,
		char const * device, char const * mountpoint,
		char const * filesystem)
{
	GtkTreeIter iter;
	GdkPixbuf * pixbuf = devices->icons[0];
	char const * ignore[] = { "kernfs", "proc", "procfs", "ptyfs" };
	char const * cdrom[] = { "/dev/cd" };
	char const * removable[] = { "/dev/sd" };
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", \"%s\", \"%s\", \"%s\")\n", __func__,
			name, device, mountpoint, filesystem);
#endif
	for(i = 0; i < sizeof(ignore) / sizeof(*ignore); i++)
		if(strcmp(ignore[i], filesystem) == 0)
			return;
	for(i = 0; i < sizeof(cdrom) / sizeof(*cdrom); i++)
		if(strncmp(cdrom[i], device, strlen(cdrom[i])) == 0)
		{
			pixbuf = devices->icons[DP_CDROM];
			break;
		}
	for(i = 0; i < sizeof(removable) / sizeof(removable); i++)
		if(strncmp(removable[i], device, strlen(removable[i])) == 0)
		{
			pixbuf = devices->icons[DP_REMOVABLE];
			break;
		}
	if(name == NULL)
	{
		if(strcmp(mountpoint, "/") == 0)
			name = _("Root filesystem");
		else
			name = mountpoint;
	}
	gtk_list_store_append(devices->store, &iter);
	gtk_list_store_set(devices->store, &iter, DC_PIXBUF, pixbuf,
			DC_NAME, name, DC_MOUNTPOINT, mountpoint, -1);
}


/* callbacks */
/* devices_on_idle */
static gboolean _devices_on_idle(gpointer data)
{
	BrowserPlugin * plugin = data;
	Devices * devices = plugin->priv;

	devices->source = 0;
	_devices_refresh(plugin, NULL);
	return FALSE;
}


/* devices_on_selection_changed */
static void _devices_on_selection_changed(gpointer data)
{
	BrowserPlugin * plugin = data;
	Devices * devices = plugin->priv;
	GtkTreeSelection * treesel;
	GtkTreeModel * model;
	GtkTreeIter iter;
	gchar * location;

	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(devices->view));
	if(gtk_tree_selection_get_selected(treesel, &model, &iter) != TRUE)
		return;
	gtk_tree_model_get(model, &iter, DC_MOUNTPOINT, &location, -1);
	plugin->helper->set_location(plugin->helper->browser, location);
	g_free(location);
}
