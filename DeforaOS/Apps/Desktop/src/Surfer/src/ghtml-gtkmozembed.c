/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Surfer */
/* Surfer is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * Surfer is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Surfer; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



#include <stdlib.h>
#include <stdio.h>
#include <gtkmozembed.h>
#include "callbacks.h"
#include "surfer.h"
#include "ghtml.h"


/* ghtml_new */
static char const * _new_get_prefs_directory(void);

GtkWidget * ghtml_new(void * data)
{
	static int init = 0;
	GtkWidget * ghtml;
	char const * buf;

	if(init == 0)
	{
		gtk_moz_embed_set_comp_path(SURFER_GTKMOZEMBED_COMPPATH);
		if((buf = _new_get_prefs_directory()) != NULL)
			gtk_moz_embed_set_profile_path(buf, "gecko");
		init = 1;
	}
	ghtml = gtk_moz_embed_new();
	/* FIXME handle callbacks in a common way */
	g_signal_connect(G_OBJECT(ghtml), "link_message", G_CALLBACK(
				on_view_link_message), data);
	g_signal_connect(G_OBJECT(ghtml), "location", G_CALLBACK(
				on_view_location), data);
	g_signal_connect(G_OBJECT(ghtml), "net_start", G_CALLBACK(
				on_view_net_start), data);
	g_signal_connect(G_OBJECT(ghtml), "net_stop", G_CALLBACK(
				on_view_net_stop), data);
	g_signal_connect(G_OBJECT(ghtml), "new_window", G_CALLBACK(
				on_view_new_window), data);
	g_signal_connect(G_OBJECT(ghtml), "progress", G_CALLBACK(
				on_view_progress), data);
	g_signal_connect(G_OBJECT(ghtml), "size_to", G_CALLBACK(on_view_resize),
			data);
	g_signal_connect(G_OBJECT(ghtml), "title", G_CALLBACK(on_view_title),
			data);
	return ghtml;
}

static char const * _new_get_prefs_directory(void)
{
	static char buf[256] = "";
	static int buf_size = sizeof(buf);
	char * home;

	if(buf[0] != '\0')
		return buf;
	if((home = getenv("HOME")) == NULL)
		return NULL;
	if(snprintf(buf, sizeof(buf), "%s/%s", home, ".surfer") >= buf_size)
		return NULL;
	return buf;
}


/* accessors */
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


/* useful */
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


/* ghtml_refresh */
void ghtml_refresh(GtkWidget * ghtml)
{
	gtk_moz_embed_reload(GTK_MOZ_EMBED(ghtml),
			GTK_MOZ_EMBED_FLAG_RELOADBYPASSCACHE);
}


/* ghtml_reload */
void ghtml_reload(GtkWidget * ghtml)
{
	gtk_moz_embed_reload(GTK_MOZ_EMBED(ghtml),
			GTK_MOZ_EMBED_FLAG_RELOADNORMAL);
}


/* ghtml_stop */
void ghtml_stop(GtkWidget * ghtml)
{
	gtk_moz_embed_stop_load(GTK_MOZ_EMBED(ghtml));
}
