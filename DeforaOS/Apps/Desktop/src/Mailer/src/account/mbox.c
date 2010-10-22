/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
/* TODO:
 * - import mails automatically from the spool to the inbox */



#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "Mailer.h"

#define min(a, b) ((a) < (b) ? (a) : (b))


/* Mbox */
/* private */
/* types */
typedef struct _Message Message;

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
	GtkTextBuffer * buffer;

	/* refresh */
	time_t mtime;
	GIOChannel * channel;
	int source;

	/* parsing */
	size_t offset;
	ParserContext context;
	Message * message;
	size_t pos; /* context-dependant */
	char * str;

	/* interface */
	char * pixbuf;
} MboxFolder;


/* constants */
#define _FOLDER_CNT 5


/* variables */
static char const _mbox_type[] = "MBOX";
static char const _mbox_name[] = "Local folders";

static char const * _error = NULL;

static AccountConfig _mbox_config[_FOLDER_CNT + 1] =
{
	{ "mbox",	"Inbox file",		ACT_FILE,	NULL },
	{ "spool",	"Spool file",		ACT_FILE,	NULL },
	{ "draft",	"Draft mails file",	ACT_FILE,	NULL },
	{ "sent",	"Sent mails file",	ACT_FILE,	NULL },
	{ "trash",	"Deleted mails file",	ACT_FILE,	NULL },
	{ NULL,		NULL,			0,		NULL }
};

static MboxFolder _mbox_inbox =
{
	&_mbox_config[0], NULL, 0, NULL,
	0, NULL, -1,
	0, PC_FROM, NULL, 0, NULL,
	"stock_inbox"
};

static MboxFolder _mbox_spool =
{
	&_mbox_config[1], NULL, 0, NULL,
	0, NULL, -1,
	0, PC_FROM, NULL, 0, NULL,
	"stock_inbox"
};

static MboxFolder _mbox_drafts =
{
	&_mbox_config[2], NULL, 0, NULL,
	0, NULL, -1,
	0, PC_FROM, NULL, 0, NULL,
	"stock_mail-handling"
};

static MboxFolder _mbox_sent =
{
	&_mbox_config[3], NULL, 0, NULL,
	0, NULL, -1,
	0, PC_FROM, NULL, 0, NULL,
	"stock_sent-mail"
};

static MboxFolder _mbox_trash =
{
	&_mbox_config[4], NULL, 0, NULL,
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


/* plug-in */
static int _mbox_init(GtkTreeStore * store, GtkTreeIter * parent,
		GtkTextBuffer * buffer);
static int _mbox_quit(void);
static GtkTextBuffer * _mbox_select(AccountFolder * folder,
		AccountMessage * message);

AccountPlugin account_plugin =
{
	NULL,
	_mbox_type,
	_mbox_name,
	_mbox_config,
	_mbox_init,
	_mbox_quit,
	_mbox_select,
	NULL
};


/* prototypes */
/* callbacks */
static gboolean _folder_idle(gpointer data);
static gboolean _folder_watch(GIOChannel * source, GIOCondition condition,
		gpointer data);


/* Message */
/* private */
/* types */
struct _Message
{
	size_t offset;
	GtkTreeIter iter;
	char ** headers;
	size_t headers_cnt;
	size_t body_offset;
	size_t body_length;
};


/* prototypes */
static Message * _message_new(off_t offset, GtkListStore * store);
static void _message_delete(Message * message);

static int _message_set_body(Message * message, off_t offset, size_t length);
static int _message_set_header(Message * message, char const * header,
		GtkListStore * store);


/* Mbox */
/* functions */
/* mbox_init */
static int _mbox_init(GtkTreeStore * store, GtkTreeIter * parent,
		GtkTextBuffer * buffer)
{
	int ret = 0;
	size_t i;
	char * filename;
	AccountFolder * af;
	MboxFolder * mbox;
	GdkPixbuf * pixbuf;
	GtkTreeIter iter;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	for(i = 0; i < _FOLDER_CNT; i++)
	{
		af = &_config_folder[i];
		mbox = af->data;
		mbox->buffer = buffer;
		filename = mbox->config->value;
		if(filename == NULL)
			continue;
		pixbuf = gtk_icon_theme_load_icon(account_plugin.helper->theme,
				(mbox->pixbuf != NULL)
				? mbox->pixbuf : "stock_folder", 16,
				0, NULL);
		gtk_tree_store_append(store, &iter, parent);
		gtk_tree_store_set(store, &iter, MF_COL_ACCOUNT, NULL,
				MF_COL_FOLDER, af, MF_COL_ICON, pixbuf,
				MF_COL_NAME, af->name, -1);
		g_object_unref(pixbuf);
		/* XXX should not be done here? */
		_config_folder[i].store = gtk_list_store_new(MH_COL_COUNT,
				G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_POINTER,
				GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING,
				G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING,
				G_TYPE_BOOLEAN, G_TYPE_INT);
		mbox->source = g_idle_add(_folder_idle, &_config_folder[i]);
	}
	return ret;
}


/* mbox_quit */
static int _mbox_quit(void)
{
	size_t i;
	MboxFolder * mf;
	size_t j;

	for(i = 0; i < _FOLDER_CNT; i++)
	{
		mf = _config_folder[i].data;
		for(j = 0; j < mf->messages_cnt; j++)
			_message_delete(mf->messages[j]);
		free(mf->messages);
		mf->messages = NULL;
		mf->messages_cnt = 0;
	}
	return 0;
}


/* mbox_select */
static GtkTextBuffer * _mbox_select(AccountFolder * folder,
		AccountMessage * message)
{
	MboxFolder * mf = folder->data;
	Message * m = (Message*)message;
	char const * filename = mf->config->value;
	GtkTextIter iter;
	FILE * fp;
	char * buf;
	size_t size;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", \"%p\")\n", __func__, folder->name,
			(void*)message);
#endif
	gtk_text_buffer_set_text(mf->buffer, "", 0);
	gtk_text_buffer_get_end_iter(mf->buffer, &iter);
	/* XXX we may still be reading the file... */
	if((fp = fopen(filename, "r")) == NULL)
		return NULL;
	if(m->body_offset != 0 && m->body_length > 0
			&& fseek(fp, m->body_offset, SEEK_SET) == 0
			&& (buf = malloc(m->body_length)) != NULL)
	{
		if((size = fread(buf, 1, m->body_length, fp)) > 0)
			gtk_text_buffer_insert(mf->buffer, &iter, buf, size);
		free(buf);
	}
	fclose(fp);
	return mf->buffer;
}


/* Message */
/* functions */
/* message_new */
static Message * _message_new(off_t offset, GtkListStore * store)
{
	Message * message;

	if((message = malloc(sizeof(*message))) == NULL)
	{
		/* FIXME catch error */
		return NULL;
	}
	message->offset = offset;
	gtk_list_store_append(store, &message->iter);
	gtk_list_store_set(store, &message->iter, MH_COL_MESSAGE, message,
			MH_COL_PIXBUF, account_plugin.helper->mail_read, -1);
	message->headers = NULL;
	message->headers_cnt = 0;
	message->body_offset = 0;
	message->body_length = 0;
	return message;
}


/* message_delete */
static void _message_delete(Message * message)
{
	size_t i;

	for(i = 0; i < message->headers_cnt; i++)
		free(message->headers[i]);
	free(message->headers);
	free(message);
}


/* message_set_body */
static int _message_set_body(Message * message, off_t offset, size_t length)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %lu, %lu)\n", __func__, (void*)message,
			offset, length);
#endif
	message->body_offset = offset;
	message->body_length = length;
	return 0;
}


/* message_set_header */
/* FIXME factorize code? */
static int _message_set_header(Message * message, char const * header,
		GtkListStore * store)
{
	/* FIXME check if the header is already set */
	char ** p;
	struct { int col; char const * name; } abc[] = {
		{ MH_COL_SUBJECT,	"Subject: "	},
		{ MH_COL_FROM,		"From: "	},
		{ MH_COL_FROM,		"From "		},
		{ MH_COL_TO,		"To: "		},
		{ MH_COL_DATE_DISPLAY,	"Date: "	},
		{ MH_COL_READ,		"Status: "	},
		{ -1,			NULL		}
	};
	size_t i;
	struct tm t;
	time_t oneday;
	char buf[20];
	gboolean read;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\", store)\n", __func__,
			(void*)message, header);
#endif
	if((p = realloc(message->headers, sizeof(*p)
					* (message->headers_cnt + 1))) == NULL)
	{
		/* FIXME catch error */
		return -1;
	}
	message->headers = p;
	if((message->headers[message->headers_cnt] = strdup(header)) == NULL)
		return -1;
	message->headers_cnt++;
	for(i = 0; abc[i].col != -1; i++)
		if(strncmp(header, abc[i].name, strlen(abc[i].name)) == 0)
			break;
	if(abc[i].col == MH_COL_DATE_DISPLAY)
	{
		oneday = time(NULL) - 86400;
		memset(&t, 0, sizeof(t));
		if(strptime(&header[6], "%a, %d %b %Y %H:%M:%S", &t) != NULL)
			strftime(buf, sizeof(buf), mktime(&t) > oneday
					? "Today %X" : "%x %X", &t);
		else
		{
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() \"%s\" failed\n",
					__func__, &header[6]);
#endif
			snprintf(buf, sizeof(buf), "%s", &header[6]);
		}
		gtk_list_store_set(store, &message->iter, MH_COL_DATE,
				mktime(&t), MH_COL_DATE_DISPLAY, buf, -1);
	}
	else if(abc[i].col == MH_COL_READ)
	{
		read = (index(&header[8], 'R') != NULL) ? TRUE : FALSE;
		gtk_list_store_set(store, &message->iter, MH_COL_READ, read,
				MH_COL_WEIGHT, PANGO_WEIGHT_BOLD, -1);
		gtk_list_store_set(store, &message->iter, MH_COL_PIXBUF, read
				? account_plugin.helper->mail_read
				: account_plugin.helper->mail_unread, -1);
	}
	else if(abc[i].col != -1)
		gtk_list_store_set(store, &message->iter, abc[i].col,
				&header[strlen(abc[i].name)], -1);
	return 0;
}

Message * _folder_message_add(AccountFolder * folder, off_t offset)
{
	MboxFolder * mbox = folder->data;
	Message ** p;
	Message * message;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", %ld)\n", __func__,
			(char const *)mbox->config->value, offset);
#endif
	if((p = realloc(mbox->messages, sizeof(*p)
					* (mbox->messages_cnt + 1))) == NULL)
	{
		/* FIXME track error */
		return NULL;
	}
	mbox->messages = p;
	if((message = _message_new(offset, folder->store)) == NULL)
	{
		/* FIXME track error */
		return NULL;
	}
	mbox->messages[mbox->messages_cnt++] = message;
	return message;
}


/* functions */
/* callbacks */
/* folder_idle */
static gboolean _folder_idle(gpointer data)
{
	AccountFolder * folder = data;
	MboxFolder * mbox = folder->data;
	struct stat st;
	char const * filename = mbox->config->value;
	GError * error = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() stat(\"%s\")\n", __func__, filename);
#endif
	if(filename[0] == '\0' || stat(filename, &st) != 0
			|| st.st_mtime == mbox->mtime)
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
	g_io_channel_set_encoding(mbox->channel, NULL, NULL);
	mbox->source = g_io_add_watch(mbox->channel, G_IO_IN, _folder_watch,
			folder);
	return FALSE;
}


/* folder_watch */
static void _watch_parse(AccountFolder * folder, char const buf[], size_t read);
static int _parse_append(AccountFolder * folder, char const buf[], size_t len);
static void _parse_from(AccountFolder * folder, char const buf[], size_t read,
		size_t * i);
static void _parse_garbage(AccountFolder * folder, char const buf[],
		size_t read, size_t * i);
static void _parse_header(AccountFolder * folder, char const buf[], size_t read,
		size_t * i);
static void _parse_body(AccountFolder * folder, char const buf[], size_t read,
		size_t * i);

static gboolean _folder_watch(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	AccountFolder * folder = data;
	MboxFolder * mbox = folder->data;
	char buf[BUFSIZ];
	size_t read;
	GError * error = NULL;
	GIOStatus status;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__,
			(char const *)mbox->config->value);
#endif
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
		/* XXX should not be necessary here */
		if(mbox->message != NULL)
			_message_set_body(mbox->message,
					mbox->message->body_offset,
					mbox->offset
					- mbox->message->body_offset);
		g_io_channel_close(source);
		mbox->channel = NULL;
		mbox->source = g_timeout_add(1000, _folder_idle, folder);
		return FALSE;
	}
	return TRUE;
}

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
		return -1; /* FIXME track error */
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
		if(mbox->message != NULL)
			mbox->context = PC_BODY;
		else
			mbox->context = PC_GARBAGE;
		return; /* switch context immediately */
	}
	for(m = 0; *i + m < read && buf[*i + m] != '\n'; m++);
	_parse_append(folder, &buf[*i], m);
	*i += m;
	if(*i == read)
		return; /* grab more data XXX is gonna force a check again */
	if(mbox->message != NULL)
		_message_set_body(mbox->message,
				mbox->message->body_offset,
				mbox->offset + *i - mbox->pos
				- mbox->message->body_offset);
	mbox->message = _folder_message_add(folder, mbox->offset + *i
			- mbox->pos);
	_message_set_header(mbox->message, mbox->str, folder->store);
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
	_parse_context(folder, (mbox->pos == 0) ? PC_FROM : PC_HEADER);
	*i = ++j;
}

static void _parse_body(AccountFolder * folder, char const buf[], size_t read,
		size_t * i)
{
	MboxFolder * mbox = folder->data;
	size_t j;

	for(j = *i; j < read && buf[j] != '\n'; j++);
	if(mbox->message->body_offset == 0)
		_message_set_body(mbox->message, mbox->offset + *i - mbox->pos,
				0);
	_parse_append(folder, &buf[*i], j - *i);
	if(j == read)
	{
		/* TODO skip data instead of storing it */
		*i = j;
		return;
	}
	_parse_context(folder, PC_FROM);
	*i = (j + 1);
}
