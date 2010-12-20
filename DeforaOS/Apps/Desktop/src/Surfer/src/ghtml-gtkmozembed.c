/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Surfer */
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



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtkmozembed.h>
#include <mozilla-config.h>
#include "callbacks.h"
#include "ghtml.h"
#include "../config.h"


/* constants */
#define xstr(s)				str(s)
#define str(s)				# s
#define SURFER_GTKMOZEMBED_COMPPATH	PREFIX "/lib/" xstr(MOZ_BUILD_APP)


/* prototypes */
/* private */
/* callbacks */
static void _on_js_status(GtkMozEmbed * view, gpointer data);
static void _on_link_message(GtkMozEmbed * view, gpointer data);
static void _on_location(GtkMozEmbed * view, gpointer data);
static void _on_net_start(GtkMozEmbed * view, gpointer data);
static void _on_net_stop(GtkMozEmbed * view, gpointer data);
static void _on_new_window(GtkMozEmbed * view, GtkMozEmbed ** ret, guint mask,
		gpointer data);
static void _on_progress(GtkMozEmbed * view, gint cur, gint max, gpointer data);
static void _on_resize(GtkMozEmbed * view, gint width, gint height,
		gpointer data);
static void _on_title(GtkMozEmbed * view, gpointer data);

/* popup */
static void _on_popup_destroy_browser(GtkMozEmbed * view, gpointer data);
static void _on_popup_resize(GtkMozEmbed * view, gint width, gint height,
		gpointer data);
static void _on_popup_title(GtkMozEmbed * view, gpointer data);


/* functions */
/* private */
/* callbacks */
static void _on_js_status(GtkMozEmbed * view, gpointer data)
{
	Surfer * surfer = data;
	char * str;

	if((str = gtk_moz_embed_get_js_status(view)) == NULL)
		return;
	if(strlen(str) > 0)
		surfer_console_message(surfer, str, NULL, -1);
	free(str);
}


static void _on_link_message(GtkMozEmbed * view, gpointer data)
{
	Surfer * surfer = data;
	char const * url;
	
	url = ghtml_get_link_message(GTK_WIDGET(view));
	surfer_set_status(surfer, url);
}


static void _on_location(GtkMozEmbed * view, gpointer data)
{
	Surfer * surfer = data;
	char const * url;

	url = ghtml_get_location(GTK_WIDGET(view));
	surfer_set_location(surfer, url);
}


static void _on_net_start(GtkMozEmbed * view, gpointer data)
{
	Surfer * surfer = data;

#if 0
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_back),
			ghtml_can_go_back(GTK_WIDGET(view)));
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_forward),
			ghtml_can_go_forward(GTK_WIDGET(view)));
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_refresh), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_stop), TRUE);
#else /* XXX hack, partially functional */
	_on_location(view, surfer);
#endif
}


static void _on_net_stop(GtkMozEmbed * view, gpointer data)
{
	Surfer * surfer = data;

#if 0
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_back),
			ghtml_can_go_back(GTK_WIDGET(view)));
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_forward),
			ghtml_can_go_forward(GTK_WIDGET(view)));
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_stop), FALSE);
#else /* XXX hack, partially functional */
	_on_location(view, surfer);
#endif
	surfer_set_status(surfer, NULL);
}


static void _on_new_window(GtkMozEmbed * view, GtkMozEmbed ** ret, guint mask,
		gpointer data)
{
	Surfer * surfer = data;
	GtkWidget * window;
	GtkWidget * vbox;
	GtkWidget * newview;

	/* FIXME this should be based on a new Surfer object */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
	gtk_window_set_title(GTK_WINDOW(window), SURFER_DEFAULT_TITLE);
	if((mask & GTK_MOZ_EMBED_FLAG_WINDOWRESIZEON)
			!= GTK_MOZ_EMBED_FLAG_WINDOWRESIZEON)
		gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
#if 0 /* FIXME disabled for now */
	if((mask & GTK_MOZ_EMBED_FLAG_MODAL)
			== GTK_MOZ_EMBED_FLAG_MODAL)
		gtk_window_set_transient_for(GTK_WINDOW(window),
				GTK_WINDOW(surfer->window));
#endif
	vbox = gtk_vbox_new(FALSE, 0);
	if((mask & GTK_MOZ_EMBED_FLAG_MENUBARON)
			!= GTK_MOZ_EMBED_FLAG_MENUBARON)
		surfer_show_menubar(surfer, FALSE);
	if((mask & GTK_MOZ_EMBED_FLAG_STATUSBARON)
			!= GTK_MOZ_EMBED_FLAG_STATUSBARON)
		surfer_show_statusbar(surfer, FALSE);
	if((mask & GTK_MOZ_EMBED_FLAG_TOOLBARON)
			!= GTK_MOZ_EMBED_FLAG_TOOLBARON)
		surfer_show_toolbar(surfer, FALSE);
	newview = gtk_moz_embed_new();
	/* XXX handle more callbacks? */
	g_signal_connect(G_OBJECT(newview), "destroy_browser", G_CALLBACK(
				_on_popup_destroy_browser), window);
	g_signal_connect(G_OBJECT(newview), "size_to", G_CALLBACK(
				_on_popup_resize), window);
	g_signal_connect(G_OBJECT(newview), "title", G_CALLBACK(
				_on_popup_title), window);
	/* FIXME other settings and callbacks */
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(newview), TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
	*ret = GTK_MOZ_EMBED(newview);
}


static void _on_progress(GtkMozEmbed * view, gint cur, gint max, gpointer data)
{
	Surfer * surfer = data;
	char buf[256];
	gdouble fraction = cur;

	if(max > 1024 || max <= 0)
		snprintf(buf, sizeof(buf), "%s%u%s%u%s", "Transferring data (",
				cur / 1024, " on ", max / 1024,
				" KB received)");
	else
		snprintf(buf, sizeof(buf), "%s%u%s%u%s", "Transferring data (",
				cur, " on ", max, " bytes received)");
	if(max > 0)
		surfer_set_progress(surfer, cur / max);
	surfer_set_status(surfer, buf);
}


static void _on_resize(GtkMozEmbed * view, gint width, gint height,
		gpointer data)
{
	Surfer * surfer = data;

	surfer_resize(surfer, width, height);
}


static void _on_title(GtkMozEmbed * view, gpointer data)
{
	Surfer * surfer = data;
	char const * title;

	title = ghtml_get_title(GTK_WIDGET(view));
	surfer_set_title(surfer, title);
}


/* popup */
static void _on_popup_destroy_browser(GtkMozEmbed * view, gpointer data)
{
	GtkWidget * window = data;

	gtk_widget_destroy(window);
}


static void _on_popup_resize(GtkMozEmbed * view, gint width, gint height,
		gpointer data)
{
	GtkWindow * window = data;

	/* FIXME probably should resize the widget itself instead */
	gtk_window_resize(window, width, height);
}


static void _on_popup_title(GtkMozEmbed * view, gpointer data)
{
	GtkWindow * window = data;
	char const * title;
	char buf[256];

	title = ghtml_get_title(GTK_WIDGET(view));
	if(title == NULL || title[0] == '\0')
		gtk_window_set_title(window, SURFER_DEFAULT_TITLE);
	else
	{
		snprintf(buf, sizeof(buf), "%s - %s", SURFER_DEFAULT_TITLE,
				title);
		gtk_window_set_title(window, buf);
	}
}


/* public */
/* ghtml_new */
static char const * _new_get_prefs_directory(void);

GtkWidget * ghtml_new(Surfer * surfer)
{
	static int init = 0;
	GtkWidget * ghtml;
	char const * buf;

	if(init == 0)
	{
		gtk_moz_embed_set_path(SURFER_GTKMOZEMBED_COMPPATH);
		if((buf = _new_get_prefs_directory()) != NULL)
			gtk_moz_embed_set_profile_path(buf, "gecko");
		init = 1;
	}
	ghtml = gtk_moz_embed_new();
	/* FIXME handle callbacks in a common way */
	g_signal_connect(G_OBJECT(ghtml), "js_status", G_CALLBACK(
				_on_js_status), surfer);
	g_signal_connect(G_OBJECT(ghtml), "link_message", G_CALLBACK(
				_on_link_message), surfer);
	g_signal_connect(G_OBJECT(ghtml), "location", G_CALLBACK(_on_location),
			surfer);
	g_signal_connect(G_OBJECT(ghtml), "net_start", G_CALLBACK(
				_on_net_start), surfer);
	g_signal_connect(G_OBJECT(ghtml), "net_stop", G_CALLBACK(_on_net_stop),
			surfer);
	g_signal_connect(G_OBJECT(ghtml), "new_window", G_CALLBACK(
				_on_new_window), surfer);
	g_signal_connect(G_OBJECT(ghtml), "progress", G_CALLBACK(_on_progress),
			surfer);
	g_signal_connect(G_OBJECT(ghtml), "size_to", G_CALLBACK(_on_resize),
			surfer);
	g_signal_connect(G_OBJECT(ghtml), "title", G_CALLBACK(_on_title),
			surfer);
	return ghtml;
}

static char const * _new_get_prefs_directory(void)
{
	static char * home = NULL;
	static char buf[256];
	const int buf_size = sizeof(buf);

	if(home != NULL)
		return buf;
	if((home = getenv("HOME")) == NULL)
		return NULL;
	if(snprintf(buf, sizeof(buf), "%s/%s", home, ".surfer") >= buf_size)
	{
		home = NULL; /* XXX will then work once it fits... */
		return NULL;
	}
	return buf;
}


/* ghtml_delete */
void ghtml_delete(GtkWidget * ghtml)
{
	/* FIXME nothing to do? */
}


/* accessors */
/* ghtml_can_go_back */
gboolean ghtml_can_go_back(GtkWidget * ghtml)
{
	return gtk_moz_embed_can_go_back(GTK_MOZ_EMBED(ghtml));
}


/* ghtml_can_go_forward */
gboolean ghtml_can_go_forward(GtkWidget * ghtml)
{
	return gtk_moz_embed_can_go_forward(GTK_MOZ_EMBED(ghtml));
}


/* ghtml_get_link_message */
char const * ghtml_get_link_message(GtkWidget * ghtml)
{
	static char buf[256];
	char * p;

	/* XXX not so elegant */
	p = gtk_moz_embed_get_link_message(GTK_MOZ_EMBED(ghtml));
	snprintf(buf, sizeof(buf), "%s", p);
	free(p);
	return buf;
}


/* ghtml_get_location */
char const * ghtml_get_location(GtkWidget * ghtml)
{
	static char buf[256];
	char * p;

	/* XXX not so elegant */
	p = gtk_moz_embed_get_location(GTK_MOZ_EMBED(ghtml));
	snprintf(buf, sizeof(buf), "%s", p);
	free(p);
	return buf;
}


/* ghtml_get_progress */
gdouble ghtml_get_progress(GtkWidget * ghtml)
{
	/* FIXME implement */
	return -1.0;
}


/* ghtml_get_security */
SurferSecurity ghtml_get_security(GtkWidget * ghtml)
{
	/* FIXME implement */
	return SS_NONE;
}


/* ghtml_get_source */
char const * ghtml_get_source(GtkWidget * widget)
{
	/* FIXME really implement */
	return NULL;
}


/* ghtml_get_status */
char const * ghtml_get_status(GtkWidget * widget)
{
	/* FIXME really implement */
	return NULL;
}


/* ghtml_get_title */
char const * ghtml_get_title(GtkWidget * ghtml)
{
	static char buf[256];
	char * p;

	/* XXX not so elegant */
	p = gtk_moz_embed_get_title(GTK_MOZ_EMBED(ghtml));
	snprintf(buf, sizeof(buf), "%s", p);
	free(p);
	return buf;
}


/* ghtml_set_proxy */
int ghtml_set_proxy(GtkWidget * ghtml, SurferProxyType type, char const * http,
		unsigned int http_port)
{
	if(type == SPT_NONE)
		return 0;
	/* FIXME really implement */
	return -error_set_code(1, "%s", strerror(ENOSYS));;
}


/* useful */
/* ghtml_execute */
void ghtml_execute(GtkWidget * ghtml, char const * code)
{
	/* FIXME implement */
}


/* ghtml_find */
gboolean ghtml_find(GtkWidget * ghtml, char const * text, gboolean sensitive,
		gboolean wrap)
{
	/* FIXME implement */
	return FALSE;
}


/* ghtml_go_back */
gboolean ghtml_go_back(GtkWidget * ghtml)
{
	if(gtk_moz_embed_can_go_back(GTK_MOZ_EMBED(ghtml)) != TRUE)
		return FALSE;
	gtk_moz_embed_go_back(GTK_MOZ_EMBED(ghtml));
	return TRUE;
}


/* ghtml_go_forward */
gboolean ghtml_go_forward(GtkWidget * ghtml)
{
	if(gtk_moz_embed_can_go_forward(GTK_MOZ_EMBED(ghtml)) != TRUE)
		return FALSE;
	gtk_moz_embed_go_forward(GTK_MOZ_EMBED(ghtml));
	return TRUE;
}


/* ghtml_load_url */
void ghtml_load_url(GtkWidget * ghtml, char const * url)
{
	gtk_moz_embed_load_url(GTK_MOZ_EMBED(ghtml), url);
}


/* ghtml_print */
void ghtml_print(GtkWidget * ghtml)
{
	/* FIXME implement */
}


/* ghtml_refresh */
void ghtml_refresh(GtkWidget * ghtml)
{
	gtk_moz_embed_reload(GTK_MOZ_EMBED(ghtml),
			GTK_MOZ_EMBED_FLAG_RELOADNORMAL);
}


/* ghtml_reload */
void ghtml_reload(GtkWidget * ghtml)
{
	gtk_moz_embed_reload(GTK_MOZ_EMBED(ghtml),
			GTK_MOZ_EMBED_FLAG_RELOADBYPASSCACHE);
}


/* ghtml_select_all */
void ghtml_select_all(GtkWidget * ghtml)
{
	/* FIXME implement */
}


/* ghtml_stop */
void ghtml_stop(GtkWidget * ghtml)
{
	gtk_moz_embed_stop_load(GTK_MOZ_EMBED(ghtml));
}


/* ghtml_unselect_all */
void ghtml_unselect_all(GtkWidget * ghtml)
{
	/* FIXME implement */
}


/* ghtml_zoom_in */
void ghtml_zoom_in(GtkWidget * ghtml)
{
	/* FIXME implement */
}


/* ghtml_zoom_out */
void ghtml_zoom_out(GtkWidget * ghtml)
{
	/* FIXME implement */
}


/* ghtml_zoom_reset */
void ghtml_zoom_reset(GtkWidget * ghtml)
{
	/* FIXME implement */
}
