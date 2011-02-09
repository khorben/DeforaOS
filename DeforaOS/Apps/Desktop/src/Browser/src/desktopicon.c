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



#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <System.h>
#include "mime.h"
#include "desktopicon.h"
#include "../config.h"
#define _(string) gettext(string)

#define PROGNAME "desktop"
#define COMMON_DND
#define COMMON_EXEC
#include "common.c"


/* constants */
#ifndef PREFIX
# define PREFIX	"/usr/local"
#endif


/* DesktopIcon */
/* types */
struct _DesktopIcon
{
	Desktop * desktop;
	char * path;
	char * name;
	gboolean isfirst;
	gboolean isdir;
	gboolean isexec;
	char const * mimetype;
	/* applications */
	char * exec;
	char * tryexec;

	/* callback */
	DesktopIconCallback callback;
	gpointer data;

	gboolean confirm;
	gboolean immutable;		/* cannot be deleted */
	gboolean selected;
	gboolean updated;		/* XXX for desktop refresh */

	GtkWidget * window;
	GtkWidget * image;
	GtkWidget * event;
	GtkWidget * label;
};


/* prototypes */
static DesktopIcon * _desktopicon_new_do(Desktop * desktop, GdkPixbuf * image,
		char const * name);

static void _desktopicon_set_icon(DesktopIcon * desktopicon, GdkPixbuf * icon);
static int _desktopicon_set_name(DesktopIcon * desktopicon, char const * name);

static void _desktopicon_update_transparency(DesktopIcon * desktopicon);

/* callbacks */
static gboolean _on_desktopicon_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static gboolean _on_icon_button_press(GtkWidget * widget,
		GdkEventButton * event, gpointer data);
static gboolean _on_icon_key_press(GtkWidget * widget, GdkEventKey * event,
		gpointer data);
static void _on_icon_drag_data_get(GtkWidget * widget, GdkDragContext * context,
		GtkSelectionData * seldata, guint info, guint time,
		gpointer data);
static void _on_icon_drag_data_received(GtkWidget * widget,
		GdkDragContext * context, gint x, gint y,
		GtkSelectionData * seldata, guint info, guint time,
		gpointer data);
static void _on_icon_open(gpointer data);
static void _on_icon_edit(gpointer data);
static void _on_icon_run(gpointer data);
static void _on_icon_open_with(gpointer data);
static void _on_icon_rename(gpointer data);
static void _on_icon_delete(gpointer data);
static void _on_icon_properties(gpointer data);


/* public */
/* functions */
/* desktopicon_new */
DesktopIcon * desktopicon_new(Desktop * desktop, char const * name,
		char const * path)
{
	DesktopIcon * desktopicon;
	struct stat st;
	Mime * mime;
	char const * mimetype = NULL;
	gboolean isdir = FALSE;
	gboolean isexec = FALSE;
	GdkPixbuf * image = NULL;
	GError * error = NULL;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\", \"%s\")\n", __func__,
			(void *)desktop, name, path);
#endif
	if(path != NULL && stat(path, &st) == 0)
	{
		mime = desktop_get_mime(desktop);
		if(S_ISDIR(st.st_mode))
		{
			isdir = TRUE;
			image = desktop_get_folder(desktop);
		}
		else if(st.st_mode & S_IXUSR)
		{
			/* FIXME use access() for this */
			isexec = TRUE;
			mime_icons(desktop_get_mime(desktop),
					desktop_get_theme(desktop),
					"application/x-executable",
					DESKTOPICON_ICON_SIZE, &image, -1);
		}
		else if((mimetype = mime_type(mime, path)) != NULL)
			mime_icons(mime, desktop_get_theme(desktop), mimetype,
					DESKTOPICON_ICON_SIZE, &image, -1);
	}
	if(name == NULL)
	{
		if((p = g_filename_to_utf8(path, -1, NULL, NULL, &error))
				== NULL)
		{
			fprintf(stderr, "%s%s\n", "desktop: ", error->message);
			name = path;
		}
		else
			name = p;
		if((name = strrchr(name, '/')) != NULL)
			name++;
	}
	if((desktopicon = _desktopicon_new_do(desktop, image, name)) == NULL)
		return NULL;
	desktopicon->isdir = isdir;
	desktopicon_set_executable(desktopicon, isexec);
	desktopicon->mimetype = mimetype;
	if(path != NULL && (desktopicon->path = strdup(path)) == NULL)
	{
		desktopicon_delete(desktopicon);
		return NULL;
	}
	return desktopicon;
}


/* desktopicon_new_application */
DesktopIcon * desktopicon_new_application(Desktop * desktop, char const * path)
{
	DesktopIcon * desktopicon;
	Config * config;
	const char section[] = "Desktop Entry";
	char const * name;
	char const * icon;
	char * exec = NULL;
	char * tryexec = NULL;
	size_t len;
	String * buf;
	GError * error = NULL;
	GdkPixbuf * image = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\")\n", __func__, (void *)desktop,
			path);
#endif
	if((config = config_new()) == NULL
			|| config_load(config, path) != 0)
	{
		if(config != NULL)
			config_delete(config);
		return NULL;
	}
	if((name = config_get(config, section, "Exec")) != NULL)
		exec = strdup(name);
	if((name = config_get(config, section, "TryExec")) != NULL)
		tryexec = strdup(name);
	if(exec == NULL || (name = config_get(config, section, "Name")) == NULL
			|| (icon = config_get(config, section, "Icon")) == NULL)
	{
		free(exec);
		free(tryexec);
		config_delete(config);
		return NULL;
	}
	/* image */
	if((len = strlen(icon)) > 4 && (strcmp(&icon[len - 4], ".png") == 0
				|| strcmp(&icon[len - 4], ".xpm") == 0)
			&& (buf = string_new_append(PREFIX, "/share/pixmaps/",
					icon, NULL)) != NULL)
	{
		image = gdk_pixbuf_new_from_file_at_size(buf,
				DESKTOPICON_ICON_SIZE, DESKTOPICON_ICON_SIZE,
				&error);
		string_delete(buf);
	}
	if(image == NULL)
		image = gtk_icon_theme_load_icon(desktop_get_theme(desktop),
				icon, DESKTOPICON_ICON_SIZE, 0, NULL);
	if(image == NULL)
		image = desktop_get_file(desktop);
	desktopicon = _desktopicon_new_do(desktop, image, name);
	config_delete(config); /* XXX also remove reference to the pixbuf */
	if(desktopicon == NULL)
	{
		free(exec);
		free(tryexec);
		return NULL;
	}
	desktopicon->exec = exec;
	desktopicon->tryexec = tryexec;
	desktopicon_set_confirm(desktopicon, FALSE);
	desktopicon_set_executable(desktopicon, TRUE);
	desktopicon_set_immutable(desktopicon, TRUE);
	return desktopicon;
}


/* desktopicon_new_category */
DesktopIcon * desktopicon_new_category(Desktop * desktop, char const * name,
		char const * icon)
{
	DesktopIcon * desktopicon;
	GdkPixbuf * image;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\", \"%s\")\n", __func__,
			(void *)desktop, name, icon);
#endif
	image = gtk_icon_theme_load_icon(desktop_get_theme(desktop), icon,
			DESKTOPICON_ICON_SIZE, 0, NULL);
	if((desktopicon = _desktopicon_new_do(desktop, image, name)) == NULL)
		return NULL;
	desktopicon_set_immutable(desktopicon, TRUE);
	return desktopicon;
}


/* desktopicon_delete */
void desktopicon_delete(DesktopIcon * desktopicon)
{
	free(desktopicon->tryexec);
	free(desktopicon->exec);
	free(desktopicon->name);
	free(desktopicon->path);
	gtk_widget_destroy(desktopicon->window);
	free(desktopicon);
}


/* accessors */
/* desktopicon_get_first */
gboolean desktopicon_get_first(DesktopIcon * desktopicon)
{
	return desktopicon->isfirst;
}


/* desktopicon_get_immutable */
gboolean desktopicon_get_immutable(DesktopIcon * desktopicon)
{
	return desktopicon->immutable;
}


/* desktopicon_get_isdir */
gboolean desktopicon_get_isdir(DesktopIcon * desktopicon)
{
	return desktopicon->isdir;
}


/* desktopicon_get_name */
char const * desktopicon_get_name(DesktopIcon * desktopicon)
{
	return desktopicon->name;
}


/* desktopicon_get_path */
char const * desktopicon_get_path(DesktopIcon * desktopicon)
{
	return desktopicon->path;
}


/* desktopicon_get_selected */
gboolean desktopicon_get_selected(DesktopIcon * desktopicon)
{
	return desktopicon->selected;
}


/* desktopicon_get_updated */
gboolean desktopicon_get_updated(DesktopIcon * desktopicon)
{
	return desktopicon->updated;
}


/* desktopicon_set_callback */
void desktopicon_set_callback(DesktopIcon * desktopicon,
		DesktopIconCallback callback, gpointer data)
{
	desktopicon->callback = callback;
	desktopicon->data = data;
}


/* desktopicon_set_confirm */
void desktopicon_set_confirm(DesktopIcon * desktopicon, gboolean confirm)
{
	desktopicon->confirm = confirm;
}


/* desktopicon_set_executable */
void desktopicon_set_executable(DesktopIcon * desktopicon, gboolean executable)
{
	desktopicon->isexec = executable;
}


/* desktopicon_set_first */
void desktopicon_set_first(DesktopIcon * desktopicon, gboolean first)
{
	desktopicon->isfirst = first;
}


/* desktopicon_set_icon */
void desktopicon_set_icon(DesktopIcon * desktopicon, GdkPixbuf * icon)
{
	_desktopicon_set_icon(desktopicon, icon);
	_desktopicon_update_transparency(desktopicon);
}


/* desktopicon_set_immutable */
void desktopicon_set_immutable(DesktopIcon * desktopicon, gboolean immutable)
{
	desktopicon->immutable = immutable;
}


/* desktopicon_set_name */
int desktopicon_set_name(DesktopIcon * desktopicon, char const * name)
{
	if(_desktopicon_set_name(desktopicon, name) != 0)
		return 1;
	_desktopicon_update_transparency(desktopicon);
	return 0;
}


/* desktopicon_set_selected */
void desktopicon_set_selected(DesktopIcon * desktopicon, gboolean selected)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %p is %s\n", (void*)desktopicon,
			selected ? "selected" : "deselected");
#endif
	desktopicon->selected = selected;
	gtk_widget_set_state(desktopicon->event, selected
			? GTK_STATE_SELECTED : GTK_STATE_NORMAL);
}


/* desktopicon_set_updated */
void desktopicon_set_updated(DesktopIcon * desktopicon, gboolean updated)
{
	desktopicon->updated = updated;
}


/* useful */
/* desktopicon_move */
void desktopicon_move(DesktopIcon * desktopicon, int x, int y)
{
	gtk_window_move(GTK_WINDOW(desktopicon->window), x, y);
}


/* desktopicon_show */
void desktopicon_show(DesktopIcon * desktopicon)
{
	gtk_widget_show_all(desktopicon->window);
}


/* private */
/* desktopicon_new_do */
static DesktopIcon * _desktopicon_new_do(Desktop * desktop, GdkPixbuf * image,
		char const * name)
{
	DesktopIcon * desktopicon;
	GtkWindow * window;
	GtkTargetEntry targets[] = { { "deforaos_browser_dnd", 0, 0 } };
	size_t targets_cnt = sizeof(targets) / sizeof(*targets);
	GtkWidget * vbox;
	GdkGeometry geometry;

	if((desktopicon = malloc(sizeof(*desktopicon))) == NULL)
		return NULL;
	memset(desktopicon, 0, sizeof(*desktopicon));
	desktopicon->desktop = desktop;
	desktopicon->confirm = TRUE;
	desktopicon->updated = TRUE;
	/* window */
	desktopicon->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	window = GTK_WINDOW(desktopicon->window);
	gtk_window_set_type_hint(window, GDK_WINDOW_TYPE_HINT_DOCK);
	gtk_window_set_resizable(window, FALSE);
	gtk_window_set_decorated(window, FALSE);
	gtk_window_set_keep_below(window, TRUE);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_focus_on_map(window, FALSE);
#endif
	g_signal_connect(G_OBJECT(desktopicon->window), "delete-event",
			G_CALLBACK(_on_desktopicon_closex), desktopicon);
	/* event */
	desktopicon->event = gtk_event_box_new();
	gtk_drag_source_set(desktopicon->event, GDK_BUTTON1_MASK, targets,
			targets_cnt, GDK_ACTION_COPY | GDK_ACTION_MOVE);
	gtk_drag_dest_set(desktopicon->event, GTK_DEST_DEFAULT_ALL, targets,
			targets_cnt, GDK_ACTION_COPY | GDK_ACTION_MOVE);
	g_signal_connect(G_OBJECT(desktopicon->event), "button-press-event",
			G_CALLBACK(_on_icon_button_press), desktopicon);
	g_signal_connect(G_OBJECT(desktopicon->event), "key-press-event",
			G_CALLBACK(_on_icon_key_press), desktopicon);
	g_signal_connect(G_OBJECT(desktopicon->event), "drag-data-get",
			G_CALLBACK(_on_icon_drag_data_get), desktopicon);
	g_signal_connect(G_OBJECT(desktopicon->event), "drag-data-received",
			G_CALLBACK(_on_icon_drag_data_received), desktopicon);
	vbox = gtk_vbox_new(FALSE, 4);
	geometry.min_width = DESKTOPICON_MIN_WIDTH;
	geometry.min_height = DESKTOPICON_MIN_HEIGHT;
	geometry.max_width = DESKTOPICON_MAX_WIDTH;
	geometry.max_height = DESKTOPICON_MAX_HEIGHT;
	geometry.base_width = DESKTOPICON_MIN_WIDTH;
	geometry.base_height = DESKTOPICON_MIN_HEIGHT;
	gtk_window_set_geometry_hints(window, vbox, &geometry, /* XXX check */
			GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE
			| GDK_HINT_BASE_SIZE);
	/* image */
	desktopicon->image = gtk_image_new();
	gtk_widget_set_size_request(desktopicon->image, DESKTOPICON_MIN_WIDTH,
			DESKTOPICON_ICON_SIZE);
	gtk_box_pack_start(GTK_BOX(vbox), desktopicon->image, FALSE, TRUE, 4);
	/* label */
	desktopicon->label = gtk_label_new(NULL);
	gtk_label_set_justify(GTK_LABEL(desktopicon->label),
			GTK_JUSTIFY_CENTER);
#if GTK_CHECK_VERSION(2, 10, 0)
	gtk_label_set_line_wrap_mode(GTK_LABEL(desktopicon->label),
			PANGO_WRAP_WORD_CHAR);
#endif
	gtk_label_set_line_wrap(GTK_LABEL(desktopicon->label), TRUE);
	gtk_box_pack_start(GTK_BOX(vbox), desktopicon->label, TRUE, FALSE, 4);
	gtk_container_add(GTK_CONTAINER(desktopicon->event), vbox);
	gtk_container_add(GTK_CONTAINER(desktopicon->window),
			desktopicon->event);
	if(image == NULL)
		image = desktop_get_file(desktop);
	_desktopicon_set_icon(desktopicon, image);
	_desktopicon_set_name(desktopicon, name);
	_desktopicon_update_transparency(desktopicon);
	return desktopicon;
}


/* desktopicon_set_icon */
static void _desktopicon_set_icon(DesktopIcon * desktopicon, GdkPixbuf * icon)
{
	gtk_image_set_from_pixbuf(GTK_IMAGE(desktopicon->image), icon);
}


/* desktopicon_set_name */
static int _desktopicon_set_name(DesktopIcon * desktopicon, char const * name)
{
	char * p;

	if((p = strdup(name)) == NULL)
		return 1;
	free(desktopicon->name);
	desktopicon->name = p;
	gtk_label_set_text(GTK_LABEL(desktopicon->label), p);
	return 0;
}


/* desktopicon_update_transparency */
static void _desktopicon_update_transparency(DesktopIcon * desktopicon)
{
	GdkPixbuf * icon;
	int width;
	int height;
	int iwidth;
	int iheight;
	GdkBitmap * mask;
	GdkBitmap * iconmask;
	GdkGC * gc;
	GdkColor black = { 0, 0, 0, 0 };
	GdkColor white = { 0xffffffff, 0xffff, 0xffff, 0xffff };
	GtkRequisition req;
	int offset;

	if((icon = gtk_image_get_pixbuf(GTK_IMAGE(desktopicon->image))) == NULL)
		return; /* XXX report error */
	gtk_window_get_size(GTK_WINDOW(desktopicon->window), &width, &height);
	iwidth = gdk_pixbuf_get_width(icon);
	iheight = gdk_pixbuf_get_height(icon);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\") window is %dx%d\n", __func__,
			desktopicon->name, width, height);
#endif
	mask = gdk_pixmap_new(NULL, width, height, 1);
	gdk_pixbuf_render_pixmap_and_mask(icon, NULL, &iconmask, 255);
	gc = gdk_gc_new(mask);
	gdk_gc_set_foreground(gc, &black);
	gdk_draw_rectangle(mask, gc, TRUE, 0, 0, width, height);
	gdk_draw_drawable(mask, gc, iconmask, 0, 0,
			(width - iwidth) / 2,
			(DESKTOPICON_ICON_SIZE + 8 - iheight) / 2, -1, -1);
	gdk_gc_set_foreground(gc, &white);
	gtk_widget_size_request(desktopicon->label, &req);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\") label is %dx%d\n", __func__,
			desktopicon->name, req.width, req.height);
#endif
	offset = DESKTOPICON_ICON_SIZE + 8;
	gdk_draw_rectangle(mask, gc, TRUE, (width - req.width - 8) / 2,
			offset + ((height - offset - req.height - 8)
				/ 2), req.width + 8, req.height + 8);
	gtk_widget_shape_combine_mask(desktopicon->window, mask, 0, 0);
	g_object_unref(gc);
	g_object_unref(iconmask);
	g_object_unref(mask);
}


/* callbacks */
static gboolean _on_desktopicon_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	DesktopIcon * di = data;

	gtk_widget_hide(widget);
	desktopicon_delete(di);
	return TRUE;
}


/* FIXME some code is duplicated from callbacks.c */
/* on_icon_button_press */
static void _popup_directory(GtkWidget * menu, DesktopIcon * desktopicon);
static void _popup_callback(GtkWidget * menu, DesktopIcon * desktopicon);
static void _popup_file(GtkWidget * menu, DesktopIcon * desktopicon);
static void _popup_mime(Mime * mime, char const * mimetype, char const * action,
		char const * label, GCallback callback, DesktopIcon * icon,
		GtkWidget * menu);

static gboolean _on_icon_button_press(GtkWidget * widget,
		GdkEventButton * event, gpointer data)
{
	DesktopIcon * desktopicon = data;
	GtkWidget * menu;
	GtkWidget * menuitem;

	if(event->state & GDK_CONTROL_MASK)
		desktopicon_set_selected(desktopicon, !desktopicon_get_selected(
					desktopicon));
	else
	{
		desktop_unselect_all(desktopicon->desktop);
		desktopicon_set_selected(desktopicon, TRUE);
	}
	if(event->type == GDK_2BUTTON_PRESS && event->button == 1)
	{
		if(desktopicon->isexec == TRUE) /* XXX slightly ugly */
			_on_icon_run(desktopicon);
		else
			_on_icon_open(desktopicon);
		return FALSE;
	}
	if(event->type != GDK_BUTTON_PRESS || event->button != 3)
		return FALSE;
	menu = gtk_menu_new();
	if(desktopicon->isdir == TRUE)
		_popup_directory(menu, desktopicon);
	else if(desktopicon->callback != NULL)
		_popup_callback(menu, desktopicon);
	else
		_popup_file(menu, desktopicon);
	if(desktopicon->immutable != TRUE)
	{
		menuitem = gtk_separator_menu_item_new();
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_DELETE,
				NULL);
		g_signal_connect_swapped(G_OBJECT(menuitem), "activate",
				G_CALLBACK(_on_icon_delete), desktopicon);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	}
	if(desktopicon->path != NULL)
	{
		menuitem = gtk_separator_menu_item_new();
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		menuitem = gtk_image_menu_item_new_from_stock(
				GTK_STOCK_PROPERTIES, NULL);
		g_signal_connect_swapped(G_OBJECT(menuitem), "activate",
				G_CALLBACK(_on_icon_properties), desktopicon);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	}
	gtk_widget_show_all(menu);
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 3, event->time);
	return TRUE;
}

static void _popup_directory(GtkWidget * menu, DesktopIcon * desktopicon)
{
	GtkWidget * menuitem;

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_icon_open), desktopicon);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_menu_item_new_with_mnemonic(_("_Rename..."));
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate",
			G_CALLBACK(_on_icon_rename), desktopicon);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
}

static void _popup_callback(GtkWidget * menu, DesktopIcon * desktopicon)
{
	GtkWidget * menuitem;

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate",
			G_CALLBACK(_on_icon_open), desktopicon);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
}

static void _popup_file(GtkWidget * menu, DesktopIcon * desktopicon)
{
	Mime * mime;
	GtkWidget * menuitem;

	mime = desktop_get_mime(desktopicon->desktop);
	_popup_mime(mime, desktopicon->mimetype, "open", GTK_STOCK_OPEN,
			G_CALLBACK(_on_icon_open), desktopicon, menu);
	_popup_mime(mime, desktopicon->mimetype, "edit",
#if GTK_CHECK_VERSION(2, 6, 0)
			GTK_STOCK_EDIT,
#else
			_("_Edit"),
#endif
			G_CALLBACK(_on_icon_edit), desktopicon, menu);
	if(desktopicon->isexec == TRUE)
	{
		menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_EXECUTE,
				NULL);
		g_signal_connect_swapped(G_OBJECT(menuitem), "activate",
				G_CALLBACK(_on_icon_run), desktopicon);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	}
	if(desktopicon->path != NULL && desktopicon->path[0] == '/')
	{
		menuitem = gtk_menu_item_new_with_mnemonic(_("Open _with..."));
		g_signal_connect_swapped(G_OBJECT(menuitem), "activate",
				G_CALLBACK(_on_icon_open_with), desktopicon);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		menuitem = gtk_separator_menu_item_new();
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		menuitem = gtk_menu_item_new_with_mnemonic(_("_Rename..."));
		g_signal_connect_swapped(G_OBJECT(menuitem), "activate",
				G_CALLBACK(_on_icon_rename), desktopicon);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	}
}

static void _popup_mime(Mime * mime, char const * mimetype, char const * action,
		char const * label, GCallback callback,
		DesktopIcon * desktopicon, GtkWidget * menu)
{
	GtkWidget * menuitem;

	if(mime_get_handler(mime, mimetype, action) == NULL)
		return;
	if(strncmp(label, "gtk-", 4) == 0)
		menuitem = gtk_image_menu_item_new_from_stock(label, NULL);
	else
		menuitem = gtk_menu_item_new_with_mnemonic(label);
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", callback,
			desktopicon);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
}

static void _on_icon_open(gpointer data)
{
	DesktopIcon * desktopicon = data;
	Mime * mime;
	char * argv[] = { "browser", "browser", "--", NULL, NULL };
	GSpawnFlags flags = G_SPAWN_SEARCH_PATH | G_SPAWN_FILE_AND_ARGV_ZERO;
	GError * error = NULL;

	if(desktopicon->path == NULL && desktopicon->callback != NULL)
	{
		desktopicon->callback(desktopicon->desktop, desktopicon->data);
		return;
	}
	if(desktopicon->isdir == FALSE)
	{
		mime = desktop_get_mime(desktopicon->desktop);
		if(mime != NULL) /* XXX ugly */
			if(mime_action(mime, "open", desktopicon->path) != 0)
				_on_icon_open_with(desktopicon);
		return;
	}
	argv[3] = desktopicon->path;
	if(g_spawn_async(NULL, argv, NULL, flags, NULL, NULL, NULL, &error)
			!= TRUE)
		desktop_error(desktopicon->desktop, argv[0], 1); /* XXX */
}

static void _on_icon_edit(gpointer data)
{
	DesktopIcon * desktopicon = data;
	Mime * mime;

	mime = desktop_get_mime(desktopicon->desktop);
	mime_action(mime, "edit", desktopicon->path);
}

static gboolean _run_confirm(DesktopIcon * desktopicon);
static void _on_icon_run(gpointer data)
{
	DesktopIcon * desktopicon = data;
	char * argv[] = { NULL, NULL, NULL };
	GSpawnFlags flags = G_SPAWN_SEARCH_PATH | G_SPAWN_FILE_AND_ARGV_ZERO;
	GError * error = NULL;

	if(desktopicon->confirm != FALSE && _run_confirm(desktopicon) != TRUE)
		return;
	if(desktopicon->tryexec != NULL) /* XXX ugly */
		argv[0] = desktopicon->tryexec;
	else if(desktopicon->exec != NULL)
		/* FIXME it's actually a format string */
		argv[0] = desktopicon->exec;
	else
		argv[0] = desktopicon->path;
	argv[1] = argv[0];
	if(g_spawn_async(NULL, argv, NULL, flags, NULL, NULL, NULL, &error)
			!= TRUE)
		desktop_error(desktopicon->desktop, argv[0], 1); /* XXX */
}

static gboolean _run_confirm(DesktopIcon * desktopicon)
{
	GtkWidget * dialog;
	int res;

	dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
			GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
			_("Warning"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s",
#endif
			_("Are you sure you want to execute this file?"));
	gtk_window_set_title(GTK_WINDOW(dialog), _("Warning"));
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return (res == GTK_RESPONSE_YES) ? TRUE : FALSE;
}

static void _on_icon_open_with(gpointer data)
{
	DesktopIcon * desktopicon = data;
	GtkWidget * dialog;
	char * filename = NULL;
	char * argv[] = { NULL, NULL, NULL, NULL };
	GSpawnFlags flags = G_SPAWN_SEARCH_PATH | G_SPAWN_FILE_AND_ARGV_ZERO;
	GError * error = NULL;

	dialog = gtk_file_chooser_dialog_new(_("Open with..."),
			GTK_WINDOW(desktopicon->window),
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename == NULL)
		return;
	argv[0] = filename;
	argv[1] = filename;
	argv[2] = desktopicon->path;
	if(g_spawn_async(NULL, argv, NULL, flags, NULL, NULL, NULL, &error)
			!= TRUE)
		desktop_error(desktopicon->desktop, argv[0], 1); /* XXX */
	g_free(filename);
}

static void _on_icon_rename(gpointer data)
{
	DesktopIcon * desktopicon = data;
	GtkWidget * dialog;
	GtkSizeGroup * group;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	int res;
	char * p;
	char * q;
	char * r;

	dialog = gtk_dialog_new_with_buttons(_("Rename"), NULL, 0,
			GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, _("Rename"),
			GTK_RESPONSE_ACCEPT, NULL);
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	vbox = GTK_DIALOG(dialog)->vbox;
#endif
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("Rename: "));
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	widget = gtk_entry_new();
	gtk_editable_set_editable(GTK_EDITABLE(widget), FALSE);
	gtk_entry_set_text(GTK_ENTRY(widget), desktopicon->name);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* entry */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("To: "));
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	widget = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(widget), desktopicon->name);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	gtk_widget_show_all(vbox);
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);
	if(res != GTK_RESPONSE_ACCEPT)
	{
		gtk_widget_destroy(dialog);
		return;
	}
	/* FIXME check errors */
	p = string_new(desktopicon->path);
	q = string_new(gtk_entry_get_text(GTK_ENTRY(widget)));
	/* FIXME convert entry from UTF-8 to filesystem's charset */
	if(q[0] == '/')
		r = string_new(q);
	else
		r = string_new_append(dirname(p), "/", q, NULL);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() rename(\"%s\", \"%s\")\n", __func__,
			desktopicon->path, r);
#else
	if(rename(desktopicon->path, r) != 0)
		desktop_error(desktopicon->desktop, r, 1);
#endif
	string_delete(p);
	string_delete(q);
	string_delete(r);
	gtk_widget_destroy(dialog);
}

static void _on_icon_delete(gpointer data)
{
	DesktopIcon * desktopicon = data;
	GtkWidget * dialog;
	unsigned long cnt = 1; /* FIXME implement */
	int res;
	GList * selection = NULL;

	/* FIXME duplicated from callbacks.c */
	dialog = gtk_message_dialog_new(GTK_WINDOW(desktopicon->window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, "%s",
			_("Warning"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(
				dialog), "%s%lu%s",
			_("Are you sure you want to delete "), cnt,
			_(" file(s)?"));
	gtk_window_set_title(GTK_WINDOW(dialog), _("Warning"));
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	if(res == GTK_RESPONSE_YES)
	{
		/* FIXME check if needs UTF-8 conversion */
		selection = g_list_append(selection, desktopicon->path);
		if(_common_exec("delete", "-ir", selection) != 0)
			desktop_error(desktopicon->desktop, "fork", 1);
		g_list_free(selection);
	}
}

static void _on_icon_properties(gpointer data)
{
	DesktopIcon * desktopicon = data;
	char * argv[] = { "properties", "properties", "--", NULL, NULL };
	GSpawnFlags flags = G_SPAWN_SEARCH_PATH | G_SPAWN_FILE_AND_ARGV_ZERO;
	GError * error = NULL;

	argv[3] = desktopicon->path;
	if(g_spawn_async(NULL, argv, NULL, flags, NULL, NULL, NULL, &error)
			!= TRUE)
		desktop_error(desktopicon->desktop, argv[0], 1); /* XXX */
}

static gboolean _on_icon_key_press(GtkWidget * widget, GdkEventKey * event,
		gpointer data)
	/* FIXME handle shift and control */
{
	DesktopIcon * desktopicon = data;

	if(event->type != GDK_KEY_PRESS)
		return FALSE;
	if(event->keyval == GDK_uparrow)
	{
		desktop_unselect_all(desktopicon->desktop);
		desktop_select_above(desktopicon->desktop, desktopicon);
	}
	else if(event->keyval == GDK_downarrow)
	{
		desktop_unselect_all(desktopicon->desktop);
		desktop_select_under(desktopicon->desktop, desktopicon);
	}
	else /* not handling it */
		return FALSE;
	return TRUE;
}

static void _on_icon_drag_data_get(GtkWidget * widget, GdkDragContext * context,
		GtkSelectionData * seldata, guint info, guint time,
		gpointer data)
{
	DesktopIcon * desktopicon = data;

	desktop_get_drag_data(desktopicon->desktop, seldata);
}

static void _on_icon_drag_data_received(GtkWidget * widget,
		GdkDragContext * context, gint x, gint y,
		GtkSelectionData * seldata, guint info, guint time,
		gpointer data)
{
	DesktopIcon * desktopicon = data;

	if(_common_drag_data_received(context, seldata, desktopicon->path) != 0)
		desktop_error(desktopicon->desktop, "fork", 1);
}
