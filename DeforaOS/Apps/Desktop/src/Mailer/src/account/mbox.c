/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Mailer */
/* Mailer is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * Mailer is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Mailer; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */
/* TODO:
 * - import mails automatically from the spool to the inbox */



#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "account.h"
/* FIXME remove this */
#include "../mailer.h"

#define min(a, b) ((a) < (b) ? (a) : (b))


/* Message */
/* private */
/* types */
typedef struct _Message
{
	off_t offset;
	GtkTreeIter iter;
	char ** headers;
	size_t headers_cnt;
} Message;


/* functions */
Message * message_new(off_t offset, GtkListStore * store)
{
	Message * message;

	if((message = malloc(sizeof(*message))) == NULL)
	{
		/* FIXME catch error */
		return NULL;
	}
	message->offset = offset;
	gtk_list_store_append(store, &message->iter);
	message->headers = NULL;
	message->headers_cnt = 0;
	return message;
}

void message_delete(Message * message)
{
	size_t i;

	for(i = 0; i < message->headers_cnt; i++)
		free(message->headers[i]);
	free(message->headers);
	free(message);
}

/* FIXME factorize code? */
int _message_set_header(Message * message, char const * header,
		GtkListStore * store)
{
	char ** p;
	struct { int col; char * name; } abc[] = {
		{ MH_COL_SUBJECT,	"Subject: "	},
		{ MH_COL_FROM,		"From: "	},
		{ MH_COL_DATE,		"Date: "	},
		{ -1,			NULL		}
	};
	size_t i;

	if((p = realloc(message->headers, sizeof(*p)
					* (message->headers_cnt + 1))) == NULL)
	{
		/* FIXME catch error */
		return 1;
	}
	message->headers = p;
	for(i = 0; abc[i].col != -1; i++)
		if(strncmp(header, abc[i].name, strlen(abc[i].name)) == 0)
			gtk_list_store_set(store, &message->iter, abc[i].col,
					&header[strlen(abc[i].name)], -1);
	if((message->headers[message->headers_cnt] = strdup(header)) == NULL)
		return 1;
	message->headers_cnt++;
	return 0;
}

typedef enum _ParserContext
{
	PC_FROM,	/* while pos < 6 not sure => fallback body or garbage */
	PC_HEADER,	/* inside a header */
	PC_BODY,	/* inside a body */
	PC_GARBAGE	/* inside crap */
} ParserContext;

typedef struct _MboxFolder
{
	AccountConfig * config;
	Message ** messages;
	size_t messages_cnt;

	/* refresh */
	time_t mtime;
	GIOChannel * channel;
	int source;

	/* parsing */
	off_t offset;
	ParserContext context;
	Message * message;
	size_t pos; /* context-dependant */
	char * str;

	/* interface */
	char * pixbuf;
} MboxFolder;

Message * _folder_message_add(AccountFolder * folder, off_t offset)
{
	MboxFolder * mbox = folder->data;
	Message ** p;
	Message * message;

	if((p = realloc(mbox->messages, sizeof(*p)
					* (mbox->messages_cnt + 1))) == NULL)
	{
		/* FIXME track error */
		return NULL;
	}
	mbox->messages = p;
	if((message = message_new(offset, folder->store)) == NULL)
	{
		/* FIXME track error */
		return NULL;
	}
	mbox->messages[mbox->messages_cnt++] = message;
	return message;
}


/* constants */
#define _FOLDER_CNT 5


/* variables */
static char const * _error = NULL;

static AccountConfig _mbox_config[_FOLDER_CNT] =
{
	{ "mbox",	"Inbox file",		ACT_FILE,	NULL },
	{ "spool",	"Incoming mails file",	ACT_FILE,	NULL },
	{ "draft",	"Draft mails file",	ACT_FILE,	NULL },
	{ "sent",	"Sent mails file",	ACT_FILE,	NULL },
	{ "trash",	"Deleted mails file",	ACT_FILE,	NULL },
};

static MboxFolder _mbox_inbox =
{
	&_mbox_config[0], NULL, 0,
	0, NULL, -1,
	0, PC_FROM, NULL, 0, NULL,
	"stock_inbox"
};

static MboxFolder _mbox_spool =
{
	&_mbox_config[1], NULL, 0,
	0, NULL, -1,
	0, PC_FROM, NULL, 0, NULL,
	"stock_inbox"
};

static MboxFolder _mbox_drafts =
{
	&_mbox_config[2], NULL, 0,
	0, NULL, -1,
	0, PC_FROM, NULL, 0, NULL,
	"stock_mail-handling"
};

static MboxFolder _mbox_sent =
{
	&_mbox_config[3], NULL, 0,
	0, NULL, -1,
	0, PC_FROM, NULL, 0, NULL,
	"stock_sent-mail"
};

static MboxFolder _mbox_trash =
{
	&_mbox_config[4], NULL, 0,
	0, NULL, -1,
	0, PC_FROM, NULL, 0, NULL,
	"stock_trash_full"
};

static AccountFolder _config_folder[_FOLDER_CNT] =
{
	{ AFT_INBOX,	"Inbox",	NULL,	&_mbox_inbox	},
	{ AFT_INBOX,	"Spool",	NULL,	&_mbox_spool	},
	{ AFT_DRAFTS,	"Drafts",	NULL,	&_mbox_drafts	},
	{ AFT_SENT,	"Sent",		NULL,	&_mbox_sent	},
	{ AFT_TRASH,	"Trash",	NULL,	&_mbox_trash	}
};


/* functions */
/* callbacks */
static gboolean _folder_idle(gpointer data);
static gboolean _folder_watch(GIOChannel * source, GIOCondition condition,
		gpointer data);


/* callbacks */
static gboolean _folder_idle(gpointer data)
{
	AccountFolder * folder = data;
	MboxFolder * mbox = folder->data;
	struct stat st;
	char const * filename = mbox->config->value;
	GError * error = NULL;

	if(stat(filename, &st) != 0 || st.st_mtime == mbox->mtime)
	{
		_error = strerror(errno);
		mbox->source = g_timeout_add(1000, _folder_idle, folder);
		return FALSE;
	}
	mbox->mtime = st.st_mtime; /* FIXME only when done */
	if(mbox->channel == NULL)
		if((mbox->channel = g_io_channel_new_file(filename, "r",
						&error)) == NULL)
	{
		_error = error->message;
		mbox->source = g_timeout_add(1000, _folder_idle, folder);
		return FALSE;
	}
	mbox->source = g_io_add_watch(mbox->channel, G_IO_IN, _folder_watch,
			folder);
	return FALSE;
}


/* folder_watch */
static void _watch_parse(AccountFolder * folder, char const buf[], size_t read);

static gboolean _folder_watch(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	AccountFolder * folder = data;
	MboxFolder * mbox = folder->data;
	char buf[BUFSIZ];
	size_t read;
	GError * error = NULL;
	GIOStatus status;

	if(condition != G_IO_IN)
		return FALSE; /* FIXME implement message deletion */
	status = g_io_channel_read_chars(source, buf, sizeof(buf), &read,
			&error);
	switch(status)
	{
		case G_IO_STATUS_ERROR:
			_error = error->message;
			/* FIXME new timeout 1000 function after invalidating
			 * mtime */
			return FALSE;
		case G_IO_STATUS_AGAIN:
			return TRUE; /* should not happen */
		case G_IO_STATUS_EOF:
		case G_IO_STATUS_NORMAL:
			break;
	}
	_watch_parse(folder, buf, read);
	if(status == G_IO_STATUS_EOF)
	{
		g_io_channel_close(source);
		mbox->channel = NULL;
		mbox->source = g_timeout_add(1000, _folder_idle, folder);
		return FALSE;
	}
	return TRUE;
}

static int _parse_append(AccountFolder * folder, char const buf[], size_t len);
static void _parse_from(AccountFolder * folder, char const buf[], size_t read,
		size_t * i);
static void _parse_garbage(AccountFolder * folder, char const buf[],
		size_t read, size_t * i);
static void _parse_header(AccountFolder * folder, char const buf[], size_t read,
		size_t * i);
static void _parse_body(AccountFolder * folder, char const buf[], size_t read,
		size_t * i);

static void _watch_parse(AccountFolder * folder, char const buf[], size_t read)
{
	MboxFolder * mbox = folder->data;
	size_t i = 0;

	while(i < read)
		switch(mbox->context)
		{
			case PC_FROM:
				_parse_from(folder, buf, read, &i);
				break;
			case PC_GARBAGE:
				_parse_garbage(folder, buf, read, &i);
				break;
			case PC_HEADER:
				_parse_header(folder, buf, read, &i);
				break;
			case PC_BODY:
				_parse_body(folder, buf, read, &i);
				break;
		}
	mbox->offset += read;
}

static int _parse_append(AccountFolder * folder, char const buf[], size_t len)
{
	MboxFolder * mbox = folder->data;
	char * p;

	if((p = realloc(mbox->str, mbox->pos + len + 1)) == NULL)
		return 1; /* FIXME track error */
	mbox->str = p;
	memcpy(&mbox->str[mbox->pos], buf, len);
	mbox->pos += len;
	mbox->str[mbox->pos] = '\0';
	return 0;
}

static void _parse_context(AccountFolder * folder, ParserContext context)
{
	MboxFolder * mbox = folder->data;

	mbox->context = context;
	free(mbox->str);
	mbox->str = NULL;
	mbox->pos = 0;
}

static void _parse_from(AccountFolder * folder, char const buf[], size_t read,
		size_t * i)
{
	static char const from[] = "From ";
	MboxFolder * mbox = folder->data;
	size_t m;

	for(m = 0; *i + m < read && m < sizeof(from) - 1 && buf[*i + m] != '\n';
			m++);
	_parse_append(folder, &buf[*i], m);
	*i += m;
	if(*i == read) /* not enough data read */
		return;
	if(mbox->pos < sizeof(from) - 1 /* early newline */
			|| strncmp(mbox->str, from, sizeof(from) - 1) != 0)
	{
		mbox->context = mbox->message != NULL
			? PC_BODY : PC_GARBAGE;
		return; /* switch context immediately */
	}
	for(m = 0; *i + m < read && buf[*i + m] != '\n'; m++);
	_parse_append(folder, &buf[*i], m);
	*i += m;
	if(*i == read)
		return; /* grab more data XXX is gonna force a check again */
	mbox->message = _folder_message_add(folder, mbox->offset + *i
			- mbox->pos);
	_parse_context(folder, PC_HEADER); /* read headers */
	(*i)++;
}

static void _parse_garbage(AccountFolder * folder, char const buf[],
		size_t read, size_t * i)
{
	for(; *i < read && buf[*i] != '\n'; (*i)++);
	if(*i == read)
		return;
	(*i)++;
	_parse_context(folder, PC_FROM);
}

static void _parse_header(AccountFolder * folder, char const buf[], size_t read,
		size_t * i)
{
	MboxFolder * mbox = folder->data;
	size_t j;

	for(j = *i; j < read && buf[j] != '\n'; j++);
	_parse_append(folder, &buf[*i], j - *i);
	*i = j;
	if(j == read)
		return;
	_message_set_header(mbox->message, mbox->str, folder->store);
	_parse_context(folder, mbox->pos == 0 ? PC_FROM : PC_HEADER);
	*i = ++j;
}

static void _parse_body(AccountFolder * folder, char const buf[], size_t read,
		size_t * i)
{
	size_t j;

	for(j = *i; j < read && buf[j] != '\n'; j++);
	if(j == read)
	{
		/* TODO skip data instead of storing it */
		_parse_append(folder, &buf[*i], j - *i);
		*i = j;
		return;
	}
	_parse_append(folder, &buf[*i], j - *i);
	_parse_context(folder, PC_FROM);
	*i = (j + 1);
}


/* Mbox */
/* private */
/* variables */
static char const _mbox_type[] = "MBOX";
static char const _mbox_name[] = "Local folders";


/* functions */
int _mbox_init(GtkTreeStore * store, GtkTreeIter * parent)
{
	int ret = 0;
	size_t i;
	char * filename;
	GtkIconTheme * theme;
	AccountFolder * af;
	MboxFolder * mbox;
	GdkPixbuf * pixbuf;
	GtkTreeIter iter;

	theme = gtk_icon_theme_get_default();
	for(i = 0; i < _FOLDER_CNT; i++)
	{
		af = &_config_folder[i];
		mbox = af->data;
		filename = mbox->config->value;
		if(filename == NULL)
			continue;
		pixbuf = gtk_icon_theme_load_icon(theme, mbox->pixbuf != NULL
				? mbox->pixbuf : "stock_folder", 16, 0, NULL);
		gtk_tree_store_append(store, &iter, parent);
		gtk_tree_store_set(store, &iter, MF_COL_ACCOUNT, NULL,
				MF_COL_FOLDER, af, MF_COL_ICON, pixbuf,
				MF_COL_NAME, af->name, -1);
		g_object_unref(pixbuf);
		/* XXX should not be done here? */
		_config_folder[i].store = gtk_list_store_new(MH_COL_COUNT,
				G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_POINTER,
				G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
				G_TYPE_STRING);
		mbox->source = g_idle_add(_folder_idle, &_config_folder[i]);
	}
	return ret;
}


int _mbox_quit(void)
{
	size_t i;
	MboxFolder * mf;
	size_t j;

	for(i = 0; i < _FOLDER_CNT; i++)
	{
		mf = _config_folder[i].data;
		for(j = 0; j < mf->messages_cnt; j++)
			message_delete(mf->messages[j]);
		free(mf->messages);
		mf->messages = NULL;
		mf->messages_cnt = 0;
	}
	return 0;
}


/* plug-in */
/* public */
AccountPlugin account_plugin =
{
	_mbox_type,
	_mbox_name,
	_mbox_config,
	_mbox_init,
	_mbox_quit
};
