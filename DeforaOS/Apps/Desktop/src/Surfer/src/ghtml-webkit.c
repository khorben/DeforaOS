/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
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



#include <webkit/webkitwebview.h>
#include "ghtml.h"


/* public */
/* functions */
/* ghtml_new */
GtkWidget * ghtml_new(Surfer * surfer)
{
	return webkit_web_view_new();
}


/* accessors */
gboolean ghtml_can_go_back(GtkWidget * ghtml)
{
	return webkit_web_view_can_go_back(WEBKIT_WEB_VIEW(ghtml));
}


gboolean ghtml_can_go_forward(GtkWidget * ghtml)
{
	return webkit_web_view_can_go_forward(WEBKIT_WEB_VIEW(ghtml));
}


char const * ghtml_get_link_message(GtkWidget * ghtml)
{
	/* FIXME implement */
	return NULL;
}


char const * ghtml_get_location(GtkWidget * ghtml)
{
	/* FIXME implement */
	return NULL;
}


char const * ghtml_get_title(GtkWidget * ghtml)
{
	/* FIXME implement */
	return NULL;
}


int ghtml_set_base(GtkWidget * ghtml, char const * url)
{
	/* FIXME implement */
	return 1;
}


/* useful */
gboolean ghtml_go_back(GtkWidget * ghtml)
{
	if(ghtml_can_go_back(ghtml) == FALSE)
		return FALSE;
	webkit_web_view_go_back(WEBKIT_WEB_VIEW(ghtml));
	return TRUE;
}


gboolean ghtml_go_forward(GtkWidget * ghtml)
{
	if(ghtml_can_go_forward(ghtml) == FALSE)
		return FALSE;
	webkit_web_view_go_forward(WEBKIT_WEB_VIEW(ghtml));
	return TRUE;
}


void ghtml_load_url(GtkWidget * ghtml, char const * url)
{
	webkit_web_view_open(WEBKIT_WEB_VIEW(ghtml), url);
}


void ghtml_refresh(GtkWidget * ghtml)
{
	webkit_web_view_reload(WEBKIT_WEB_VIEW(ghtml));
}


void ghtml_reload(GtkWidget * ghtml)
{
	return ghtml_refresh(ghtml);
}


void ghtml_stop(GtkWidget * ghtml)
{
	webkit_web_view_stop_loading(WEBKIT_WEB_VIEW(ghtml));
}


void ghtml_select_all(GtkWidget * ghtml)
{
	webkit_web_view_select_all(WEBKIT_WEB_VIEW(ghtml));
}


void ghtml_unselect_all(GtkWidget * ghtml)
{
	/* FIXME implement */
}


void ghtml_zoom_in(GtkWidget * ghtml)
{
	webkit_web_view_zoom_in(WEBKIT_WEB_VIEW(ghtml));
}


void ghtml_zoom_out(GtkWidget * ghtml)
{
	webkit_web_view_zoom_out(WEBKIT_WEB_VIEW(ghtml));
}


void ghtml_zoom_reset(GtkWidget * ghtml)
{
	webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(ghtml), 1.0);
}
