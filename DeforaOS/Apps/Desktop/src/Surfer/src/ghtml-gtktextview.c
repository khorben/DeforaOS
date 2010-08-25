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
#include <errno.h>
#include <libintl.h>
#include <System/Parser/XML.h>
#include "ghtml.h"
#include "../config.h"
#include "common/conn.c"
#include "common/history.c"
#include "common/url.c"
#define _(string) gettext(string)


/* private */
/* types */
typedef struct _GHtml
{
	Surfer * surfer;
	char const * title;

	/* history */
	GList * history;
	GList * current;

	/* search */
	size_t search;

	/* connection */
	Conn * conn;
	char * buffer;
	size_t buffer_cnt;

	/* html widget */
	GtkWidget * view;
	GtkTextBuffer * tbuffer;
} GHtml;


/* prototypes */
static int _ghtml_document_load(GHtml * ghtml, char const * url,
		char const * post);
static int _ghtml_stop(GHtml * ghtml);


/* public */
/* functions */
/* ghtml_new */
GtkWidget * ghtml_new(Surfer * surfer)
{
	GHtml * ghtml;
	GtkWidget * widget;

	if((ghtml = malloc(sizeof(*ghtml))) == NULL)
		return NULL;
	ghtml->surfer = surfer;
	ghtml->title = NULL;
	ghtml->history = NULL;
	ghtml->current = NULL;
	ghtml->conn = NULL;
	ghtml->buffer = NULL;
	ghtml->buffer_cnt = 0;
	widget = gtk_scrolled_window_new(NULL, NULL);
	g_object_set_data(G_OBJECT(widget), "ghtml", ghtml);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	ghtml->view = gtk_text_view_new();
	ghtml->tbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ghtml->view));
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(ghtml->view),
			FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(ghtml->view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(ghtml->view),
			GTK_WRAP_WORD_CHAR);
	gtk_container_add(GTK_CONTAINER(widget), ghtml->view);
	return widget;
}


/* ghtml_delete */
void ghtml_delete(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	free(ghtml->buffer);
	if(ghtml->conn != NULL)
		_conn_delete(ghtml->conn);
	free(ghtml);
}


/* accessors */
/* ghtml_can_go_back */
gboolean ghtml_can_go_back(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	return _history_can_go_back(ghtml->current);
}


/* ghtml_can_go_forward */
gboolean ghtml_can_go_forward(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	return _history_can_go_forward(ghtml->current);
}


/* ghtml_get_link_message */
char const * ghtml_get_link_message(GtkWidget * widget)
{
	/* FIXME implement */
	return NULL;
}


/* ghtml_get_location */
char const * ghtml_get_location(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	return _history_get_location(ghtml->current);
}


/* ghtml_get_progress */
gdouble ghtml_get_progress(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	if(ghtml->conn == NULL)
		return -1.0;
	return _conn_get_progress(ghtml->conn);
}


/* ghtml_get_source */
char const * ghtml_get_source(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	return ghtml->buffer;
}


/* ghtml_get_status */
char const * ghtml_get_status(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	if(ghtml->conn == NULL)
		return NULL;
	return _conn_get_status(ghtml->conn);
}


/* ghtml_get_title */
char const * ghtml_get_title(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	return ghtml->title;
}


/* useful */
/* ghtml_execute */
void ghtml_execute(GtkWidget * ghtml, char const * code)
{
	/* FIXME implement */
}


/* ghtml_find */
static char const * _find_string(char const * big, char const * little,
		gboolean sensitive);
static gboolean _find_match(GHtml * ghtml, char const * buf, char const * str,
		size_t tlen);

gboolean ghtml_find(GtkWidget * widget, char const * text, gboolean sensitive,
		gboolean wrap)
{
	gboolean ret = FALSE;
	GHtml * ghtml;
	size_t tlen;
	GtkTextIter start;
	GtkTextIter end;
	gchar * buf;
	size_t blen;
	char const * str;

	if(text == NULL || (tlen = strlen(text)) == 0)
		return ret;
	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	/* XXX highly inefficient */
	gtk_text_buffer_get_start_iter(ghtml->tbuffer, &start);
	gtk_text_buffer_get_end_iter(ghtml->tbuffer, &end);
	buf = gtk_text_buffer_get_text(ghtml->tbuffer, &start, &end, FALSE);
	if(buf == NULL || (blen = strlen(buf)) == 0)
		return ret;
	if(ghtml->search >= blen)
		ghtml->search = 0;
	if((str = _find_string(&buf[ghtml->search], text, sensitive)) != NULL)
		ret = _find_match(ghtml, buf, str, tlen);
	else if(wrap && ghtml->search != 0) /* wrap around */
	{
		buf[ghtml->search] = '\0';
		if((str = _find_string(buf, text, sensitive)) != NULL)
			ret = _find_match(ghtml, buf, str, tlen);
	}
	g_free(buf);
	return ret;
}

static char const * _find_string(char const * big, char const * little,
		gboolean sensitive)
{
	return sensitive ? strstr(big, little) : strcasestr(big, little);
}

static gboolean _find_match(GHtml * ghtml, char const * buf, char const * str,
		size_t tlen)
{
	size_t offset;
	GtkTextIter start;
	GtkTextIter end;

	offset = str - buf;
	ghtml->search = offset + 1;
	gtk_text_buffer_get_iter_at_offset(ghtml->tbuffer, &start, offset);
	gtk_text_buffer_get_iter_at_offset(ghtml->tbuffer, &end, offset + tlen);
	gtk_text_buffer_select_range(ghtml->tbuffer, &start, &end);
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(ghtml->view), &start, 0.0,
			FALSE, 0.0, 0.0);
	return TRUE;
}


/* ghtml_go_back */
gboolean ghtml_go_back(GtkWidget * ghtml)
{
	/* FIXME implement */
	return FALSE;
}


/* ghtml_go_forward */
gboolean ghtml_go_forward(GtkWidget * ghtml)
{
	/* FIXME implement */
	return FALSE;
}


/* ghtml_load_url */
void ghtml_load_url(GtkWidget * widget, char const * url)
{
	GHtml * ghtml;
	gchar * link;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	if((link = _ghtml_make_url(NULL, url)) != NULL)
		url = link;
	_ghtml_document_load(ghtml, url, NULL);
	g_free(link);
}


/* ghtml_print */
void ghtml_print(GtkWidget * ghtml)
{
	/* FIXME implement */
}


/* ghtml_refresh */
void ghtml_refresh(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	_ghtml_stop(ghtml);
	/* FIXME give ghtml directly, forgets POST */
	ghtml_load_url(widget, _history_get_location(ghtml->current));
}


/* ghtml_reload */
void ghtml_reload(GtkWidget * ghtml)
{
	ghtml_refresh(ghtml);
}


/* ghtml_stop */
void ghtml_stop(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	_ghtml_stop(ghtml);
}


/* ghtml_select_all */
void ghtml_select_all(GtkWidget * widget)
{
	GHtml * ghtml;
	GtkTextIter start;
	GtkTextIter end;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	gtk_text_buffer_get_start_iter(ghtml->tbuffer, &start);
	gtk_text_buffer_get_end_iter(ghtml->tbuffer, &end);
	gtk_text_buffer_select_range(ghtml->tbuffer, &start, &end);
}


/* ghtml_unselect_all */
void ghtml_unselect_all(GtkWidget * widget)
{
	GHtml * ghtml;
	GtkTextIter start;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	gtk_text_buffer_get_start_iter(ghtml->tbuffer, &start);
	gtk_text_buffer_select_range(ghtml->tbuffer, &start, &start);
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


/* functions */
static ssize_t _document_load_write(Conn * conn, char const * buf, size_t size,
		gpointer data);
static gboolean _document_load_idle(gpointer data);
static void _document_load_write_node(GHtml * ghtml, XMLNode * node);

static int _ghtml_document_load(GHtml * ghtml, char const * url,
		char const * post)
{
	History * h;

	_ghtml_stop(ghtml);
	if((h = _history_new(url, post)) == NULL)
		return 1;
	ghtml->history = g_list_append(ghtml->history, h);
	ghtml->current = g_list_last(ghtml->history);
	gtk_text_buffer_set_text(ghtml->tbuffer, "", 0);
	free(ghtml->buffer);
	ghtml->buffer = NULL;
	ghtml->buffer_cnt = 0;
	ghtml->search = 0;
	surfer_set_location(ghtml->surfer, url);
	surfer_set_title(ghtml->surfer, NULL);
	if((ghtml->conn = _conn_new(ghtml->surfer, url, post)) == NULL)
		return 1;
	_conn_set_callback_write(ghtml->conn, _document_load_write, ghtml);
	g_idle_add(_document_load_idle, ghtml);
	return 0;
}

static ssize_t _document_load_write(Conn * conn, char const * buf, size_t size,
		gpointer data)
{
	GHtml * ghtml = data;
	XML * xml;
	XMLDocument * doc;
	char * p;

	if(size == 0)
	{
		if((xml = xml_new_string(NULL, ghtml->buffer,
						ghtml->buffer_cnt)) == NULL)
			return 0;
		if((doc = xml_get_document(xml)) != NULL)
			_document_load_write_node(ghtml, doc->root);
		xml_delete(xml);
	}
	if((p = realloc(ghtml->buffer, ghtml->buffer_cnt + size)) == NULL)
		return -error_set_code(1, "%s", strerror(errno));
	ghtml->buffer = p;
	memcpy(&ghtml->buffer[ghtml->buffer_cnt], buf, size);
	ghtml->buffer_cnt += size;
	return size;
}

static void _document_load_write_node(GHtml * ghtml, XMLNode * node)
{
	size_t i;
	GtkTextIter iter;

	if(node == NULL)
		return;
	switch(node->type)
	{
		case XML_NODE_TYPE_DATA:
			/* FIXME looks like memory corruption at some point */
			gtk_text_buffer_get_end_iter(ghtml->tbuffer, &iter);
			gtk_text_buffer_insert(ghtml->tbuffer, &iter,
					node->data.buffer, node->data.size);
			break;
		case XML_NODE_TYPE_TAG:
			for(i = 0; i < node->tag.childs_cnt; i++)
				_document_load_write_node(ghtml,
						node->tag.childs[i]);
			break;
	}
}

static gboolean _document_load_idle(gpointer data)
{
	GHtml * ghtml = data;

	if(ghtml->conn != NULL)
		_conn_load(ghtml->conn);
	return FALSE;
}


/* ghtml_stop */
static int _ghtml_stop(GHtml * ghtml)
{
	if(ghtml->conn == NULL)
		return 0;
	_conn_delete(ghtml->conn);
	ghtml->conn = NULL;
	return 0;
}
