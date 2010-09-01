/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
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



#include <glib.h>


/* History */
/* private */
/* types */
typedef struct _History
{
	gchar * base;
	gchar * url;
	gchar * post;
} History;


/* prototypes */
static History * _history_new(gchar const * url, gchar const * post);
static void _history_delete(History * h);

/* accessors */
static gboolean _history_can_go_back(GList * history);
static gboolean _history_can_go_forward(GList * history);

static gchar const * _history_get_location(GList * history);


/* functions */
/* history_new */
static History * _history_new(gchar const * url, gchar const * post)
{
	History * h;

	if((h = object_new(sizeof(*h))) == NULL)
		return NULL;
	h->base = NULL;
	h->url = g_strdup(url);
	h->post = (post != NULL) ? g_strdup(post) : NULL;
	if(h->url == NULL || (post != NULL && h->post == NULL))
	{
		_history_delete(h);
		return NULL;
	}
	return h;
}


/* history_delete */
static void _history_delete(History * h)
{
	g_free(h->base);
	g_free(h->url);
	g_free(h->post);
	free(h);
}


/* accessors */
/* history_can_go_back */
static gboolean _history_can_go_back(GList * history)
{
	return history != NULL && history->prev != NULL
		? TRUE : FALSE;
}


/* history_can_go_forward */
static gboolean _history_can_go_forward(GList * history)
{
	return history != NULL && history->next != NULL
		? TRUE : FALSE;
}


/* history_get_location */
static gchar const * _history_get_location(GList * history)
{
	History * ghh;

	if(history == NULL)
		return NULL;
	if((ghh = history->data) == NULL)
		return NULL;
	return ghh->url;
}


/* useful */
static GList * _history_append(History * history, GList * list)
{
	GList * l;

	if(list == NULL)
		return g_list_append(list, history);
	for(l = list->next; l != NULL; l = g_list_delete_link(l, l))
		_history_delete(l->data);
	list->next = NULL;
	l = g_list_append(list, history);
	return l->next;
}
