/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Mailer */
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



#include <stdarg.h>
#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <string.h>
#include <time.h>
#include <System.h>
#include "mailer.h"
#include "message.h"


/* Message */
/* private */
/* types */
typedef struct _MessageHeader
{
	char * header;
	char * value;
} MessageHeader;

struct _Message
{
	GtkListStore * store;
	GtkTreeRowReference * row;

	MessageHeader * headers;
	size_t headers_cnt;

	GtkTextBuffer * body;

	AccountMessage * data;
};


/* prototypes */
/* accessors */
static gboolean _message_get_iter(Message * message, GtkTreeIter * iter);

static gboolean _message_set(Message * message, ...);
static int _message_set_date(Message * message, char const * date);
static int _message_set_status(Message * message, char const * status);

/* useful */
/* message headers */
static int _message_header_set(MessageHeader * mh, char const * header,
		char const * value);


/* constants */
static struct
{
	char const * header;
	MailerHeaderColumn column;
	int (*callback)(Message * message, char const * value);
} _message_columns[] =
{
	{ "Date",	0,			_message_set_date	},
	{ "From",	MHC_FROM,		NULL			},
	{ "Status",	0,			_message_set_status	},
	{ "Subject",	MHC_SUBJECT,		NULL			},
	{ "To",		MHC_TO,			NULL			},
	{ NULL,		0,			NULL			}
};


/* public */
/* functions */
/* message_new */
Message * message_new(AccountMessage * message, GtkListStore * store,
		GtkTreeIter * iter)
{
	Message * ret;
	GtkTreePath * path;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %p, %p)\n", __func__, message, store,
			iter);
#endif
	if((ret = object_new(sizeof(*ret))) == NULL)
		return NULL;
	ret->store = store;
	path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), iter);
	ret->row = gtk_tree_row_reference_new(GTK_TREE_MODEL(store), path);
	gtk_tree_path_free(path);
	ret->headers = NULL;
	ret->headers_cnt = 0;
	ret->body = gtk_text_buffer_new(NULL);
	ret->data = message;
	gtk_list_store_set(store, iter, MHC_MESSAGE, ret, -1);
	_message_set_date(ret, NULL);
	_message_set_status(ret, NULL);
	return ret;
}


/* message_delete */
void message_delete(Message * message)
{
	gtk_tree_row_reference_free(message->row);
	free(message->headers);
	object_delete(message);
}


/* accessors */
/* message_get_body */
GtkTextBuffer * message_get_body(Message * message)
{
	return message->body;
}


/* message_get_data */
AccountMessage * message_get_data(Message * message)
{
	return message->data;
}


/* message_get_header */
char const * message_get_header(Message * message, char const * header)
{
	size_t i;

	for(i = 0; i < message->headers_cnt; i++)
		if(strcmp(message->headers[i].header, header) == 0)
			return message->headers[i].value;
	return NULL;
}


/* message_get_source */
GtkTextBuffer * message_get_source(Message * message)
{
	/* FIXME implement */
	return NULL;
}


/* message_set_body */
int message_set_body(Message * message, char const * buf, size_t cnt,
		gboolean append)
{
	GtkTextIter iter;

	if(buf == NULL)
		buf = "";
	if(append != TRUE)
		gtk_text_buffer_set_text(message->body, "", 0);
	/* FIXME:
	 * - check encoding
	 * - parse MIME, etc... */
	gtk_text_buffer_get_end_iter(message->body, &iter);
	gtk_text_buffer_insert(message->body, &iter, buf, cnt);
	return 0;
}


/* message_set_header */
int message_set_header(Message * message, char const * header)
{
	int ret;
	size_t i;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\")\n", __func__, (void*)message,
			header);
#endif
	if(header == NULL)
		return -1;
	for(i = 0; header[i] != '\0' && header[i] != ':'; i++);
	if(header[i] == '\0' || header[i + 1] != ' ')
		return -1;
	if((p = malloc(i + 1)) == NULL)
		return -1;
	snprintf(p, i + 1, "%s", header);
	ret = message_set_header_value(message, p, &header[i + 2]);
	free(p);
	return ret;
}


/* message_set_header_value */
int message_set_header_value(Message * message, char const * header,
		char const * value)
{
	size_t i;
	MessageHeader * p;
	MailerHeaderColumn column;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\", \"%s\")\n", __func__,
			(void*)message, header, value);
#endif
	/* FIXME remove the header when value == NULL */
	for(i = 0; i < message->headers_cnt; i++)
		if(strcmp(message->headers[i].header, header) == 0)
			break;
	if(i == message->headers_cnt)
	{
		if((p = realloc(message->headers, sizeof(*p)
						* (message->headers_cnt + 1)))
				== NULL)
			return -1;
		message->headers = p;
		p = &message->headers[message->headers_cnt];
		memset(p, 0, sizeof(*p));
		if(_message_header_set(p, header, value) != 0)
			return -1;
		message->headers_cnt++;
	}
	else if(_message_header_set(&message->headers[i], NULL, value) != 0)
		return -1;
	/* FIXME parse/convert input */
	for(i = 0; _message_columns[i].header != NULL; i++)
	{
		if(strcmp(_message_columns[i].header, header) != 0)
			continue;
		if((column = _message_columns[i].column) != 0)
			_message_set(message, column, value, -1);
		if(_message_columns[i].callback == NULL)
			return 0;
		return _message_columns[i].callback(message, value);
	}
	return 0;
}


/* message_set_read */
void message_set_read(Message * message, gboolean read)
{
	char const * status;
	char * p;
	size_t i;

	if((status = message_get_header(message, "Status")) == NULL)
	{
		message_set_header(message, read ? "Status: RO" : "Status: O");
		return;
	}
	if(!read)
	{
		if((p = strdup(status)) == NULL)
			return; /* XXX report error */
		for(i = 0; p[i] != '\0' && p[i] != 'R'; i++);
		if(p[i] == 'R')
			for(; p[i] != '\0'; i++)
				p[i] = p[i + 1];
		message_set_header_value(message, "Status", p);
		free(p);
	}
	else if(strchr(status, 'R') == NULL)
	{
		i = strlen(status);
		if((p = malloc(i + 2)) == NULL)
			return; /* XXX report error */
		snprintf(p, i + 2, "%c%s", 'R', status);
		message_set_header_value(message, "Status", p);
		free(p);
	}
}


/* private */
/* functions */
/* accessors */
/* message_get */
static gboolean _message_get_iter(Message * message, GtkTreeIter * iter)
{
	GtkTreePath * path;

	if((path = gtk_tree_row_reference_get_path(message->row)) == NULL)
		return FALSE;
	return gtk_tree_model_get_iter(GTK_TREE_MODEL(message->store), iter,
			path);
}


/* message_set */
static gboolean _message_set(Message * message, ...)
{
	va_list ap;
	GtkTreeIter iter;

	if(_message_get_iter(message, &iter) != TRUE)
		return FALSE;
	va_start(ap, message);
	gtk_list_store_set_valist(message->store, &iter, ap);
	va_end(ap);
	return TRUE;
}


/* message_set_date */
static int _message_set_date(Message * message, char const * date)
{
	struct tm tm;
	time_t t;
	char buf[32];

	if(date != NULL && strptime(date, "%a, %d %b %Y %T", &tm) != NULL)
		t = mktime(&tm);
	else
	{
		t = time(NULL);
		gmtime_r(&t, &tm);
	}
	strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", &tm);
	_message_set(message, MHC_DATE, t, MHC_DATE_DISPLAY, buf, -1);
	return 0;
}


/* message_set_status */
static int _message_set_status(Message * message, char const * status)
{
	gboolean read;
	GtkIconTheme * theme;
	GdkPixbuf * pixbuf;

	read = (status == NULL || strchr(status, 'R') != NULL) ? TRUE : FALSE;
	theme = gtk_icon_theme_get_default();
	pixbuf = gtk_icon_theme_load_icon(theme, read ? "mail-read"
			: "mail-unread", 16, 0, NULL);
	_message_set(message, MHC_READ, read, MHC_WEIGHT, (read)
			? PANGO_WEIGHT_NORMAL : PANGO_WEIGHT_BOLD,
			MHC_PIXBUF, pixbuf, -1);
	return 0;
}


/* useful */
/* message_header */
static int _message_header_set(MessageHeader * mh, char const * header,
		char const * value)
{
	int ret = 0;
	char * h = NULL;
	char * v = NULL;

	if(header != NULL && (h = strdup(header)) == NULL)
		ret |= -1;
	if(value != NULL && (v = strdup(value)) == NULL)
		ret |= -1;
	if(ret != 0)
		return ret;
	if(h != NULL)
	{
		free(mh->header);
		mh->header = h;
	}
	if(v != NULL)
	{
		free(mh->value);
		mh->value = v;
	}
	return 0;
}
