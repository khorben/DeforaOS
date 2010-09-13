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
typedef enum _GHtmlDisplay
{
	GHTML_DISPLAY_BLOCK,
	GHTML_DISPLAY_INLINE
} GHtmlDisplay;

typedef enum _GHtmlPosition
{
	GHTML_POSITION_BEFORE = 0,
	GHTML_POSITION_HEAD,
	GHTML_POSITION_HEAD_TITLE,
	GHTML_POSITION_BODY,
	GHTML_POSITION_AFTER
} GHtmlPosition;

typedef struct _GHtmlProperty
{
	char const * name;
	void * value;
} GHtmlProperty;

typedef struct _GHtmlTag
{
	char const * name;
	GHtmlDisplay display;
	GHtmlProperty const * properties;
	GtkTextTag * tag;
} GHtmlTag;

#define GHTML_TAGS_COUNT 24
typedef struct _GHtml
{
	Surfer * surfer;
	char * title;

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
	GHtmlTag tags[GHTML_TAGS_COUNT];
	GtkTextTag * tag;

	/* parsing */
	GHtmlPosition position;
} GHtml;


/* constants */
/* properties */
static const GHtmlProperty _ghtml_properties_a[] =
{
	{ "foreground", "blue" },
	{ "underline", (void*)PANGO_UNDERLINE_SINGLE },
	{ "underline-set", (void*)TRUE },
	{ NULL, NULL }
};

static const GHtmlProperty _ghtml_properties_b[] =
{
	{ "weight", (void*)PANGO_WEIGHT_BOLD },
	{ "weight-set", (void*)TRUE },
	{ NULL, NULL }
};

static const GHtmlProperty _ghtml_properties_del[] =
{
	{ "strikethrough", (void*)TRUE },
	{ "strikethrough-set", (void*)TRUE },
	{ NULL, NULL }
};

/* XXX should use "scale" but gdouble values are not accepted this way */
static const GHtmlProperty _ghtml_properties_h1[] =
{
	{ "font", "Sans 16" },
	{ "weight", (void*)PANGO_WEIGHT_BOLD },
	{ "weight-set", (void*)TRUE },
	{ NULL, NULL }
};

static const GHtmlProperty _ghtml_properties_h2[] =
{
	{ "font", "Sans 14" },
	{ "weight", (void*)PANGO_WEIGHT_BOLD },
	{ "weight-set", (void*)TRUE },
	{ NULL, NULL }
};

static const GHtmlProperty _ghtml_properties_h3[] =
{
	{ "font", "Sans 13" },
	{ "weight", (void*)PANGO_WEIGHT_BOLD },
	{ "weight-set", (void*)TRUE },
	{ NULL, NULL }
};

static const GHtmlProperty _ghtml_properties_h4[] =
{
	{ "font", "Sans 12" },
	{ "weight", (void*)PANGO_WEIGHT_BOLD },
	{ "weight-set", (void*)TRUE },
	{ NULL, NULL }
};

static const GHtmlProperty _ghtml_properties_h5[] =
{
	{ "font", "Sans 11" },
	{ "weight", (void*)PANGO_WEIGHT_BOLD },
	{ "weight-set", (void*)TRUE },
	{ NULL, NULL }
};

static const GHtmlProperty _ghtml_properties_h6[] =
{
	{ "font", "Sans 10" },
	{ "weight", (void*)PANGO_WEIGHT_BOLD },
	{ "weight-set", (void*)TRUE },
	{ NULL, NULL }
};

static const GHtmlProperty _ghtml_properties_i[] =
{
	{ "style", (void*)PANGO_STYLE_ITALIC },
	{ "style-set", (void*)TRUE },
	{ NULL, NULL }
};

static const GHtmlProperty _ghtml_properties_pre[] =
{
	{ "family", "Monospace" },
	{ "wrap-mode", (void*)GTK_WRAP_NONE },
	{ "wrap-mode-set", (void*)TRUE },
	{ NULL, NULL }
};

static const GHtmlProperty _ghtml_properties_tt[] =
{
	{ "family", "Monospace" },
	{ NULL, NULL }
};

static const GHtmlProperty _ghtml_properties_u[] =
{
	{ "underline", (void*)PANGO_UNDERLINE_SINGLE },
	{ "underline-set", (void*)TRUE },
	{ NULL, NULL }
};

/* tags */
static const GHtmlTag _ghtml_tags[GHTML_TAGS_COUNT] =
{
	{ "a", GHTML_DISPLAY_INLINE,	_ghtml_properties_a,	NULL	},
	{ "b", GHTML_DISPLAY_INLINE,	_ghtml_properties_b,	NULL	},
	{ "del", GHTML_DISPLAY_INLINE,	_ghtml_properties_del,	NULL	},
	{ "div", GHTML_DISPLAY_BLOCK,	NULL,			NULL	},
	{ "em", GHTML_DISPLAY_INLINE,	_ghtml_properties_b,	NULL	},
	{ "form", GHTML_DISPLAY_BLOCK,	NULL,			NULL	},
	{ "h1", GHTML_DISPLAY_BLOCK,	_ghtml_properties_h1,	NULL	},
	{ "h2", GHTML_DISPLAY_BLOCK,	_ghtml_properties_h2,	NULL	},
	{ "h3", GHTML_DISPLAY_BLOCK,	_ghtml_properties_h3,	NULL	},
	{ "h4", GHTML_DISPLAY_BLOCK,	_ghtml_properties_h4,	NULL	},
	{ "h5", GHTML_DISPLAY_BLOCK,	_ghtml_properties_h5,	NULL	},
	{ "h6", GHTML_DISPLAY_BLOCK,	_ghtml_properties_h6,	NULL	},
	{ "hr", GHTML_DISPLAY_BLOCK,	NULL,			NULL	},
	{ "i", GHTML_DISPLAY_INLINE,	_ghtml_properties_i,	NULL	},
	{ "li", GHTML_DISPLAY_BLOCK,	NULL,			NULL	},
	{ "ol", GHTML_DISPLAY_BLOCK,	NULL,			NULL	},
	{ "p", GHTML_DISPLAY_BLOCK,	NULL,			NULL	},
	{ "pre", GHTML_DISPLAY_BLOCK,_ghtml_properties_pre,	NULL	},
	{ "s", GHTML_DISPLAY_INLINE,	_ghtml_properties_del,	NULL	},
	{ "strike", GHTML_DISPLAY_INLINE,_ghtml_properties_del,	NULL	},
	{ "strong", GHTML_DISPLAY_INLINE,_ghtml_properties_b,	NULL	},
	{ "tt", GHTML_DISPLAY_INLINE,	_ghtml_properties_tt,	NULL	},
	{ "u", GHTML_DISPLAY_INLINE,	_ghtml_properties_u,	NULL	},
	{ "ul", GHTML_DISPLAY_BLOCK,	NULL,			NULL	}
};


/* prototypes */
static int _ghtml_document_load(GHtml * ghtml, char const * url,
		char const * post);
static int _ghtml_stop(GHtml * ghtml);

/* callbacks */
static gboolean _on_view_event_after(GtkWidget * widget, GdkEvent * event,
		gpointer data);


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
	g_signal_connect(G_OBJECT(ghtml->view), "event-after", G_CALLBACK(
				_on_view_event_after), ghtml);
	ghtml->tbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ghtml->view));
	memcpy(ghtml->tags, _ghtml_tags, sizeof(_ghtml_tags));
	ghtml->tag = NULL;
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
	if(ghtml->conn != NULL)
		_conn_delete(ghtml->conn);
	free(ghtml->title);
	free(ghtml->buffer);
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
gboolean ghtml_go_back(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	if(ghtml->current == NULL || ghtml->current->prev == NULL)
		return FALSE;
	ghtml->current = ghtml->current->prev;
	ghtml_load_url(widget, _history_get_location(ghtml->current));
	return TRUE;
}


/* ghtml_go_forward */
gboolean ghtml_go_forward(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	if(ghtml->current == NULL || ghtml->current->next == NULL)
		return FALSE;
	ghtml->current = ghtml->current->next;
	ghtml_load_url(widget, _history_get_location(ghtml->current));
	return TRUE;
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
static void _document_load_write_node_tag(GHtml * ghtml, XMLNodeTag * node);
static void _document_load_write_node_tag_link(GHtml * ghtml,
		XMLNodeTag * node);

static int _ghtml_document_load(GHtml * ghtml, char const * url,
		char const * post)
{
	char const * q;
	History * h;

	_ghtml_stop(ghtml);
	if((q = _history_get_location(ghtml->current)) == NULL
			|| strcmp(q, url) != 0)
	{
		if((h = _history_new(url, post)) == NULL)
			return 1;
		ghtml->current = _history_append(h, ghtml->current);
		ghtml->history = g_list_first(ghtml->current);
	}
	gtk_text_buffer_set_text(ghtml->tbuffer, "", 0);
	free(ghtml->buffer);
	ghtml->buffer = NULL;
	ghtml->buffer_cnt = 0;
	ghtml->search = 0;
	surfer_set_location(ghtml->surfer, url);
	free(ghtml->title);
	ghtml->title = NULL;
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
	XMLPrefs prefs;
	XML * xml;
	XMLDocument * doc;
	char * p;

	memset(&prefs, 0, sizeof(prefs));
	prefs.filters |= XML_FILTER_WHITESPACE;
	if(size == 0)
	{
		if((xml = xml_new_string(&prefs, ghtml->buffer,
						ghtml->buffer_cnt)) == NULL)
			return 0;
		ghtml->position = GHTML_POSITION_BEFORE;
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
	GtkTextIter iter;

	if(node == NULL)
		return;
	switch(node->type)
	{
		case XML_NODE_TYPE_DATA:
			if(ghtml->position == GHTML_POSITION_HEAD_TITLE)
			{
				free(ghtml->title);
				ghtml->title = strdup(node->data.buffer);
				surfer_set_title(ghtml->surfer, ghtml->title);
				break;
			}
			else if(ghtml->position != GHTML_POSITION_BODY)
				break;
			/* FIXME looks like memory corruption at some point */
			gtk_text_buffer_get_end_iter(ghtml->tbuffer, &iter);
			gtk_text_buffer_insert_with_tags(ghtml->tbuffer, &iter,
					node->data.buffer, node->data.size,
					ghtml->tag, NULL);
			break;
		case XML_NODE_TYPE_ENTITY: /* XXX ignore for now */
			break;
		case XML_NODE_TYPE_TAG:
			_document_load_write_node_tag(ghtml, &node->tag);
			break;
	}
}

static void _document_load_write_node_tag(GHtml * ghtml, XMLNodeTag * node)
{
	size_t i;
	GHtmlDisplay display = GHTML_DISPLAY_INLINE;
	GtkTextIter iter;
	GHtmlProperty const * p;

	ghtml->tag = NULL;
	for(i = 0; i < GHTML_TAGS_COUNT; i++)
		if(strcmp(ghtml->tags[i].name, node->name) == 0)
			break;
	if(i < GHTML_TAGS_COUNT)
	{
		display = ghtml->tags[i].display;
		ghtml->tag = ghtml->tags[i].tag;
		if(ghtml->tag == NULL && ghtml->tags[i].properties != NULL)
		{
			ghtml->tag = gtk_text_buffer_create_tag(
					ghtml->tbuffer, NULL, NULL);
			p = ghtml->tags[i].properties;
			if(strcmp(node->name, "a") != 0)
				ghtml->tags[i].tag = ghtml->tag;
			else
				_document_load_write_node_tag_link(ghtml, node);
			for(i = 0; p[i].name != NULL; i++)
				g_object_set(G_OBJECT(ghtml->tag), p[i].name,
						p[i].value, NULL);
		}
	}
	if(strcmp(node->name, "head") == 0)
		ghtml->position = GHTML_POSITION_HEAD;
	else if(strcmp(node->name, "body") == 0)
		ghtml->position = GHTML_POSITION_BODY;
	else if(strcmp(node->name, "title") == 0)
	{
		if(ghtml->position == GHTML_POSITION_HEAD)
			ghtml->position = GHTML_POSITION_HEAD_TITLE;
	}
	if(display == GHTML_DISPLAY_BLOCK)
	{
		gtk_text_buffer_get_end_iter(ghtml->tbuffer, &iter);
		gtk_text_buffer_insert(ghtml->tbuffer, &iter, "\n", 1);
		for(i = 0; i < node->childs_cnt; i++)
			_document_load_write_node(ghtml, node->childs[i]);
		gtk_text_buffer_get_end_iter(ghtml->tbuffer, &iter);
		gtk_text_buffer_insert(ghtml->tbuffer, &iter, "\n", 1);
	}
	else
		for(i = 0; i < node->childs_cnt; i++)
			_document_load_write_node(ghtml, node->childs[i]);
	if(strcmp(node->name, "head") == 0)
		ghtml->position = GHTML_POSITION_BEFORE;
	else if(strcmp(node->name, "body") == 0)
		ghtml->position = GHTML_POSITION_AFTER;
	else if(strcmp(node->name, "title") == 0)
	{
		if(ghtml->position == GHTML_POSITION_HEAD_TITLE)
			ghtml->position = GHTML_POSITION_HEAD;
	}
}

static void _document_load_write_node_tag_link(GHtml * ghtml, XMLNodeTag * node)
{
	size_t i;

	for(i = 0; i < node->attributes_cnt; i++)
		if(strcmp(node->attributes[i]->name, "href") == 0)
			break;
	if(i < node->attributes_cnt)
		g_object_set_data(G_OBJECT(ghtml->tag), "link", strdup(
					node->attributes[i]->value));
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


/* callbacks */
/* on_view_event_after */
static gboolean _on_view_event_after(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	GHtml * ghtml = data;
	GdkEventButton * eb;
	GtkTextIter start;
	GtkTextIter end;
	gint x;
	gint y;
	GtkTextIter iter;
	GSList * tags;
	GSList * p;
	char * link = NULL;

	if(event->type != GDK_BUTTON_RELEASE || event->button.button != 1)
		return FALSE;
	eb = &event->button;
	gtk_text_buffer_get_selection_bounds(ghtml->tbuffer, &start, &end);
	if(gtk_text_iter_get_offset(&start) != gtk_text_iter_get_offset(&end))
		return FALSE;
	gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(widget),
			GTK_TEXT_WINDOW_WIDGET, eb->x, eb->y, &x, &y);
	gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(widget), &iter, x, y);
	tags = gtk_text_iter_get_tags(&iter);
	for(p = tags; p != NULL; p = p->next)
		if((link = g_object_get_data(G_OBJECT(p->data), "link"))
				!= NULL)
			break;
	if(tags != NULL)
		g_slist_free(tags);
	if(link != NULL)
		surfer_open(ghtml->surfer, link); /* XXX support relative */
	return FALSE;
}
