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
/* TODO:
 * - import mails automatically from the spool to the inbox */



#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <glib.h>
#include "Mailer.h"

#define min(a, b) ((a) < (b) ? (a) : (b))


/* Mbox */
/* private */
#define _FOLDER_CNT 4


/* types */
typedef enum _ParserContext
{
	PC_FROM,	/* while pos < 6 not sure => fallback body or garbage */
	PC_HEADER,	/* inside a header */
	PC_BODY,	/* inside a body */
	PC_GARBAGE	/* inside crap */
} ParserContext;

typedef struct _Mbox Mbox;

struct _AccountMessage
{
	Message * message;

	/* Mbox */
	size_t offset;
	size_t body_offset;
	size_t body_length;
};

struct _AccountFolder
{
	Folder * folder;

	/* Mbox */
	Mbox * mbox;
	AccountConfig * config;
	AccountMessage ** messages;
	size_t messages_cnt;

	/* refresh */
	time_t mtime;
	GIOChannel * channel;
	int source;

	/* parsing */
	size_t offset;
	ParserContext context;
	AccountMessage * message;
	size_t pos; /* context-dependant */
	char * str;

	/* interface */
	char * pixbuf;
};

struct _Mbox
{
	AccountPlugin * plugin;

	AccountFolder folders[_FOLDER_CNT];

	/* refresh */
	unsigned int timeout;
};


/* constants */
#define MBOX_REFRESH_TIMEOUT	5000

static AccountConfig _mbox_config[] =
{
	{ "mbox",	"Inbox file",		ACT_FILE,	NULL },
	{ "spool",	"Spool file",		ACT_FILE,	NULL },
	{ "draft",	"Draft mails file",	ACT_FILE,	NULL },
	{ "sent",	"Sent mails file",	ACT_FILE,	NULL },
	{ "trash",	"Deleted mails file",	ACT_FILE,	NULL },
	{ NULL,		NULL,			0,		NULL }
};

static const struct
{
	FolderType type;
	char const * name;
	char const * icon;
	unsigned int config;
} _mbox_folder_defaults[_FOLDER_CNT] =
{
	{ FT_INBOX,	"Inbox",	"stock_inbox",	0 },
	{ FT_DRAFTS,	"Drafts",	"stock_drafts",	2 },
	{ FT_SENT,	"Sent",		"stock_sent",	3 },
	{ FT_TRASH,	"Trash",	"stock_trash",	4 }
};


/* plug-in */
static int _mbox_init(AccountPlugin * plugin);
static int _mbox_destroy(AccountPlugin * plugin);
static char * _mbox_get_source(AccountPlugin * plugin, AccountFolder * folder,
		AccountMessage * message);
static int _mbox_refresh(AccountPlugin * plugin, AccountFolder * folder,
		AccountMessage * message);

AccountPlugin account_plugin =
{
	NULL,
	"MBOX",
	"Local folders",
	NULL,
	_mbox_config,
	_mbox_init,
	_mbox_destroy,
	_mbox_get_source,
	_mbox_refresh,
	NULL
};


/* prototypes */
/* callbacks */
static gboolean _folder_idle(gpointer data);
static gboolean _folder_watch(GIOChannel * source, GIOCondition condition,
		gpointer data);


/* AccountMessage */
/* private */
/* prototypes */
static AccountMessage * _message_new(Folder * folder, off_t offset);
static void _message_delete(AccountMessage * message);

static int _message_set_body(AccountMessage * message, off_t offset,
		size_t length);
static int _message_set_header(AccountMessage * message, char const * header);


/* Mbox */
/* functions */
/* mbox_init */
static int _mbox_init(AccountPlugin * plugin)
{
	Mbox * mbox;
	size_t i;
	AccountFolder * af;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((mbox = calloc(1, sizeof(*mbox))) == NULL)
		return -1;
	plugin->priv = mbox;
	mbox->plugin = plugin;
	mbox->timeout = MBOX_REFRESH_TIMEOUT;
	for(i = 0; i < _FOLDER_CNT; i++)
	{
		af = &mbox->folders[i];
		af->config = &_mbox_config[_mbox_folder_defaults[i].config];
		if(af->config->value == NULL)
			continue;
		af->folder = plugin->helper->folder_new(plugin->helper->account,
				af, NULL, _mbox_folder_defaults[i].type,
				_mbox_folder_defaults[i].name);
		af->mbox = mbox;
		af->source = g_idle_add(_folder_idle, af);
	}
	return 0;
}


/* mbox_destroy */
static int _mbox_destroy(AccountPlugin * plugin)
{
	Mbox * mbox = plugin->priv;
	size_t i;
	AccountFolder * mf;
	size_t j;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(mbox == NULL) /* XXX _mbox_destroy() may be called uninitialized */
		return 0;
	for(i = 0; i < _FOLDER_CNT; i++)
	{
		mf = &mbox->folders[i];
		for(j = 0; j < mf->messages_cnt; j++)
			_message_delete(mf->messages[j]);
		free(mf->messages);
		mf->messages = NULL;
		mf->messages_cnt = 0;
	}
	free(mbox);
	return 0;
}


/* mbox_get_source */
static char * _mbox_get_source(AccountPlugin * plugin, AccountFolder * folder,
		AccountMessage * message)
{
	char * ret = NULL;
	char const * filename = folder->config->value;
	FILE * fp;
	size_t len;

	if(message->body_offset < message->offset)
		return NULL;
	if((fp = fopen(filename, "r")) == NULL)
	{
		plugin->helper->error(plugin->helper->account, filename, 1);
		return NULL;
	}
	len = message->body_offset - message->offset + message->body_length;
	if(fseek(fp, message->offset, SEEK_SET) == 0
			&& (ret = malloc(len + 1)) != NULL
			&& fread(ret, sizeof(*ret), len, fp) == len)
		ret[len] = '\0';
	else
		free(ret);
	if(fclose(fp) != 0)
	{
		plugin->helper->error(plugin->helper->account, filename, 1);
		free(ret);
		ret = NULL;
	}
	return ret;
}


/* mbox_refresh */
static int _mbox_refresh(AccountPlugin * plugin, AccountFolder * folder,
		AccountMessage * message)
{
	char const * filename = folder->config->value;
	FILE * fp;
	char * buf;
	size_t size;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %p)\n", __func__, (void*)folder,
			(void*)message);
#endif
	if(message == NULL)
		return 0;
	plugin->helper->message_set_body(message->message, NULL, 0, 0);
	/* XXX we may still be reading the file... */
	if((fp = fopen(filename, "r")) == NULL)
		return -plugin->helper->error(NULL, strerror(errno), 1);
	if(message->body_offset != 0 && message->body_length > 0
			&& fseek(fp, message->body_offset, SEEK_SET) == 0
			&& (buf = malloc(message->body_length)) != NULL)
	{
		if((size = fread(buf, 1, message->body_length, fp)) > 0)
			plugin->helper->message_set_body(message->message, buf,
					size, 1);
		free(buf);
	}
	fclose(fp);
	return 0;
}


/* AccountMessage */
/* functions */
/* message_new */
static AccountMessage * _message_new(Folder * folder, off_t offset)
{
	AccountMessage * message;

	if((message = malloc(sizeof(*message))) == NULL)
	{
		/* FIXME catch error */
		return NULL;
	}
	message->message = account_plugin.helper->message_new(
			account_plugin.helper->account, folder, message);
	message->offset = offset;
	message->body_offset = 0;
	message->body_length = 0;
	if(message->message == NULL)
	{
		_message_delete(message);
		return NULL;
	}
	return message;
}


/* message_delete */
static void _message_delete(AccountMessage * message)
{
	account_plugin.helper->message_delete(message->message);
	free(message);
}


/* message_set_body */
static int _message_set_body(AccountMessage * message, off_t offset,
		size_t length)
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
static int _message_set_header(AccountMessage * message, char const * header)
{
	return account_plugin.helper->message_set_header(message->message,
			header);
}


/* functions */
/* callbacks */
/* folder_idle */
static gboolean _folder_idle(gpointer data)
{
	AccountFolder * folder = data;
	Mbox * mbox = folder->mbox;
	struct stat st;
	char const * filename = folder->config->value;
	GError * error = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() stat(\"%s\")\n", __func__, filename);
#endif
	if(filename == NULL || filename[0] == '\0')
		return FALSE;
	if(stat(filename, &st) != 0)
	{
		mbox->plugin->helper->error(NULL, strerror(errno), 1);
		folder->source = g_timeout_add(mbox->timeout, _folder_idle,
				folder);
		return FALSE;
	}
	if(st.st_mtime == folder->mtime)
	{
		folder->source = g_timeout_add(mbox->timeout, _folder_idle,
				folder);
		return FALSE;
	}
	folder->mtime = st.st_mtime; /* FIXME only when done */
	if(folder->channel == NULL)
		if((folder->channel = g_io_channel_new_file(filename, "r",
						&error)) == NULL)
	{
		mbox->plugin->helper->error(NULL, error->message, 1);
		folder->source = g_timeout_add(mbox->timeout, _folder_idle,
				folder);
		return FALSE;
	}
	g_io_channel_set_encoding(folder->channel, NULL, NULL);
	folder->source = g_io_add_watch(folder->channel, G_IO_IN, _folder_watch,
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
static AccountMessage * _folder_message_add(AccountFolder * folder,
		off_t offset);

static gboolean _folder_watch(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	AccountFolder * folder = data;
	Mbox * mbox = folder->mbox;
	char buf[BUFSIZ];
	size_t read;
	GError * error = NULL;
	GIOStatus status;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__,
			(char const *)folder->config->value);
#endif
	if(condition != G_IO_IN)
		return FALSE; /* FIXME implement message deletion */
	status = g_io_channel_read_chars(source, buf, sizeof(buf), &read,
			&error);
	switch(status)
	{
		case G_IO_STATUS_ERROR:
			mbox->plugin->helper->error(NULL, error->message, 1);
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
		if(folder->message != NULL)
			_message_set_body(folder->message,
					folder->message->body_offset,
					folder->offset
					- folder->message->body_offset);
		g_io_channel_close(source);
		folder->channel = NULL;
		folder->source = g_timeout_add(mbox->timeout, _folder_idle,
				folder);
		return FALSE;
	}
	return TRUE;
}

static void _watch_parse(AccountFolder * folder, char const buf[], size_t read)
{
	size_t i = 0;

	while(i < read)
		switch(folder->context)
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
	folder->offset += read;
}

static int _parse_append(AccountFolder * folder, char const buf[], size_t len)
{
	char * p;

	if((p = realloc(folder->str, folder->pos + len + 1)) == NULL)
		return -1; /* FIXME track error */
	folder->str = p;
	memcpy(&folder->str[folder->pos], buf, len);
	folder->pos += len;
	folder->str[folder->pos] = '\0';
	return 0;
}

static void _parse_context(AccountFolder * folder, ParserContext context)
{
	folder->context = context;
	free(folder->str);
	folder->str = NULL;
	folder->pos = 0;
}

static void _parse_from(AccountFolder * folder, char const buf[], size_t read,
		size_t * i)
{
	static char const from[] = "From ";
	size_t m;

	for(m = 0; *i + m < read && m < sizeof(from) - 1 && buf[*i + m] != '\n';
			m++);
	_parse_append(folder, &buf[*i], m);
	*i += m;
	if(*i == read) /* not enough data read */
		return;
	if(folder->pos < sizeof(from) - 1 /* early newline */
			|| strncmp(folder->str, from, sizeof(from) - 1) != 0)
	{
		if(folder->message != NULL)
			folder->context = PC_BODY;
		else
			folder->context = PC_GARBAGE;
		return; /* switch context immediately */
	}
	for(m = 0; *i + m < read && buf[*i + m] != '\n'; m++);
	_parse_append(folder, &buf[*i], m);
	*i += m;
	if(*i == read)
		return; /* grab more data XXX is gonna force a check again */
	if(folder->message != NULL)
		_message_set_body(folder->message,
				folder->message->body_offset,
				folder->offset + *i - folder->pos
				- folder->message->body_offset);
	folder->message = _folder_message_add(folder, folder->offset + *i
			- folder->pos);
	_message_set_header(folder->message, folder->str);
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
	size_t j;

	for(j = *i; j < read && buf[j] != '\n'; j++);
	_parse_append(folder, &buf[*i], j - *i);
	*i = j;
	if(j == read)
		return;
	_message_set_header(folder->message, folder->str);
	_parse_context(folder, (folder->pos == 0) ? PC_FROM : PC_HEADER);
	*i = ++j;
}

static void _parse_body(AccountFolder * folder, char const buf[], size_t read,
		size_t * i)
{
	size_t j;

	for(j = *i; j < read && buf[j] != '\n'; j++);
	if(folder->message->body_offset == 0)
		_message_set_body(folder->message, folder->offset + *i
				- folder->pos, 0);
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

static AccountMessage * _folder_message_add(AccountFolder * folder,
		off_t offset)
{
	AccountMessage ** p;
	AccountMessage * message;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", %ld)\n", __func__,
			(char const *)folder->config->value, offset);
#endif
	if((p = realloc(folder->messages, sizeof(*p)
					* (folder->messages_cnt + 1))) == NULL)
	{
		/* FIXME track error */
		return NULL;
	}
	folder->messages = p;
	if((message = _message_new(folder->folder, offset)) == NULL)
	{
		/* FIXME track error */
		return NULL;
	}
	folder->messages[folder->messages_cnt++] = message;
	return message;
}
