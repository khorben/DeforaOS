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
/* FIXME:
 * - recovery on errors
 * - really queue commands with callbacks
 * - send keep-alives? (NOOP)
 * - support multiple connections? */



#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <glib.h>
#include <System.h>
#include "Mailer.h"


/* POP3 */
/* private */
/* types */
struct _AccountFolder
{
	Folder * folder;

	AccountMessage ** messages;
	size_t messages_cnt;
};

struct _AccountMessage
{
	Message * message;

	unsigned int id;
};

typedef enum _POP3Context
{
	P3C_INIT = 0,
	P3C_AUTHORIZATION_USER,
	P3C_AUTHORIZATION_PASS,
	P3C_TRANSACTION_LIST,
	P3C_TRANSACTION_RETR,
	P3C_TRANSACTION_STAT,
	P3C_TRANSACTION_TOP
} POP3Context;

typedef struct _POP3
{
	int fd;
	guint source;

	AccountFolder inbox;
	AccountFolder trash;

	GIOChannel * channel;
	char * rd_buf;
	size_t rd_buf_cnt;
	guint rd_source;
	char * wr_buf;
	size_t wr_buf_cnt;
	guint wr_source;
	POP3Context context;
	unsigned int id;
	gboolean body;
	AccountMessage * message;
} POP3;


/* variables */
static char const _pop3_type[] = "POP3";
static char const _pop3_name[] = "POP3 server";

static AccountConfig _pop3_config[] =
{
	{ "username",	"Username",		ACT_STRING,	NULL },
	{ "password",	"Password",		ACT_PASSWORD,	NULL },
	{ "hostname",	"Server hostname",	ACT_STRING,	NULL },
	{ "port",	"Server port",		ACT_UINT16,	NULL },
#if 0 /* FIXME SSL is not supported yet */
	{ "ssl",	"Use SSL",		ACT_BOOLEAN,	NULL },
#endif
	{ "delete",	"Delete read mails on server",
						ACT_BOOLEAN,	NULL },
	{ NULL,		NULL,			ACT_NONE,	NULL }
};


/* prototypes */
static int _pop3_init(AccountPlugin * plugin);
static int _pop3_destroy(AccountPlugin * plugin);
static int _pop3_refresh(AccountPlugin * plugin, AccountFolder * folder,
		AccountMessage * message);

/* useful */
static int _pop3_command(AccountPlugin * plugin, char const * command);
static int _pop3_parse(AccountPlugin * plugin);

static AccountMessage * _pop3_message_get(AccountPlugin * plugin,
		AccountFolder * folder, unsigned int id);
static AccountMessage * _pop3_message_new(AccountPlugin * plugin,
		AccountFolder * folder, unsigned int id);
static void _pop3_message_delete(AccountPlugin * plugin,
		AccountMessage * message);

/* callbacks */
static gboolean _on_idle(gpointer data);
static gboolean _on_watch_can_read(GIOChannel * source, GIOCondition condition,
		gpointer data);
static gboolean _on_watch_can_write(GIOChannel * source, GIOCondition condition,
		gpointer data);


/* public */
/* variables */
AccountPlugin account_plugin =
{
	NULL,
	_pop3_type,
	_pop3_name,
	NULL,
	_pop3_config,
	_pop3_init,
	_pop3_destroy,
	NULL,
	_pop3_refresh,
	NULL
};


/* private */
/* functions */
/* pop3_init */
static int _pop3_init(AccountPlugin * plugin)
{
	POP3 * pop3;

	if((pop3 = malloc(sizeof(*pop3))) == NULL)
		return -1;
	plugin->priv = pop3;
	pop3->fd = -1;
	pop3->source = g_idle_add(_on_idle, plugin);
	pop3->inbox.folder = plugin->helper->folder_new(plugin->helper->account,
			&pop3->inbox, NULL, FT_INBOX, "Inbox");
	pop3->inbox.messages = NULL;
	pop3->inbox.messages_cnt = 0;
	pop3->trash.folder = plugin->helper->folder_new(plugin->helper->account,
			&pop3->trash, NULL, FT_TRASH, "Trash");
	pop3->trash.messages = NULL;
	pop3->trash.messages_cnt = 0;
	pop3->channel = NULL;
	pop3->rd_buf = NULL;
	pop3->rd_buf_cnt = 0;
	pop3->rd_source = 0;
	pop3->wr_buf = NULL;
	pop3->wr_buf_cnt = 0;
	pop3->wr_source = 0;
	pop3->context = P3C_INIT;
	pop3->id = 0;
	pop3->message = NULL;
	return 0;
}


/* pop3_destroy */
static int _pop3_destroy(AccountPlugin * plugin)
{
	POP3 * pop3 = plugin->priv;
	GError * error = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(pop3 == NULL) /* XXX _pop3_destroy() may be called uninitialized */
		return 0;
	if(pop3->rd_source != 0)
		g_source_remove(pop3->rd_source);
	free(pop3->rd_buf);
	if(pop3->wr_source != 0)
		g_source_remove(pop3->wr_source);
	free(pop3->wr_buf);
	if(pop3->source != 0)
		g_source_remove(pop3->source);
	if(pop3->channel != NULL)
	{
		g_io_channel_shutdown(pop3->channel, TRUE, &error);
		g_io_channel_unref(pop3->channel);
	}
	if(pop3->fd >= 0)
		close(pop3->fd);
	free(pop3);
	return 0;
}


/* pop3_refresh */
static int _pop3_refresh(AccountPlugin * plugin, AccountFolder * folder,
		AccountMessage * message)
{
	POP3 * pop3 = plugin->priv;
	char buf[32];

	snprintf(buf, sizeof(buf), "%s %u", "RETR", message->id);
	pop3->context = P3C_TRANSACTION_RETR;
	pop3->id = message->id;
	return _pop3_command(plugin, buf);
}


/* useful */
/* pop3_command */
static int _pop3_command(AccountPlugin * plugin, char const * command)
{
	POP3 * pop3 = plugin->priv;
	size_t len;
	char * p;

	if(command == NULL || (len = strlen(command)) == 0)
		return -1;
	len += 2;
	if((p = realloc(pop3->wr_buf, pop3->wr_buf_cnt + len + 1)) == NULL)
		return -1;
	pop3->wr_buf = p;
	snprintf(&pop3->wr_buf[pop3->wr_buf_cnt], len + 1, "%s%s", command,
			"\r\n");
	pop3->wr_buf_cnt += len;
	/* FIXME may not be connected */
	if(pop3->wr_source == 0)
		pop3->wr_source = g_io_add_watch(pop3->channel, G_IO_OUT,
				_on_watch_can_write, plugin);
	return 0;
}


/* pop3_parse */
static int _parse_context(AccountPlugin * plugin, char const * answer);

static int _pop3_parse(AccountPlugin * plugin)
{
	int ret = 0;
	POP3 * pop3 = plugin->priv;
	size_t i;
	size_t j;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	for(i = 0, j = 0;; j = ++i)
	{
		for(; i < pop3->rd_buf_cnt; i++)
			if(pop3->rd_buf[i] == '\r' && i + 1 < pop3->rd_buf_cnt
					&& pop3->rd_buf[++i] == '\n')
				break;
		if(i == pop3->rd_buf_cnt)
			break;
		pop3->rd_buf[i - 1] = '\0';
		/* FIXME this string may be found inside mail content */
		if(strncmp("-ERR", &pop3->rd_buf[j], 4) == 0)
			ret |= -plugin->helper->error(plugin->helper->account,
					&pop3->rd_buf[j + 4], 1);
		else
			ret |= _parse_context(plugin, &pop3->rd_buf[j]);
	}
	if(j != 0)
	{
		pop3->rd_buf_cnt -= j;
		memmove(pop3->rd_buf, &pop3->rd_buf[j], pop3->rd_buf_cnt);
	}
	return ret;
}

static int _parse_context(AccountPlugin * plugin, char const * answer)
{
	int ret;
	POP3 * pop3 = plugin->priv;
	char const * p;
	char * q;
	unsigned int u;
	unsigned int v;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, answer);
#endif
	switch(pop3->context)
	{
		case P3C_INIT:
			pop3->context = P3C_AUTHORIZATION_USER;
			if((p = plugin->config[0].value) != NULL)
			{
				q = g_strdup_printf("%s %s", "USER", p);
				ret = _pop3_command(plugin, q);
				free(q);
				return ret;
			}
		case P3C_AUTHORIZATION_USER:
			pop3->context = P3C_AUTHORIZATION_PASS;
			if((p = plugin->config[1].value) != NULL)
			{
				q = g_strdup_printf("%s %s", "PASS", p);
				ret = _pop3_command(plugin, q);
				free(q);
				return ret;
			}
		case P3C_AUTHORIZATION_PASS:
			pop3->context = P3C_TRANSACTION_STAT;
			return _pop3_command(plugin, "STAT");
		case P3C_TRANSACTION_LIST:
			if(strcmp(answer, ".") == 0)
			{
				pop3->context = P3C_TRANSACTION_TOP;
				return 0;
			}
			if(sscanf(answer, "%u %u", &u, &v) != 2)
				return -1;
			pop3->id = u;
			/* FIXME may not be supported by the server */
			q = g_strdup_printf("%s %u 0", "TOP", u);
			ret = _pop3_command(plugin, q);
			free(q);
			return ret;
		case P3C_TRANSACTION_RETR:
		case P3C_TRANSACTION_TOP: /* same as RETR without body */
			/* FIXME this string may be found inside mail content */
			if(strncmp(answer, "+OK", 3) == 0)
			{
				pop3->body = FALSE;
				pop3->message = _pop3_message_get(plugin,
						&pop3->inbox, pop3->id);
				return 0;
			}
			if(strcmp(answer, ".") == 0)
				return 0;
			if(answer[0] == '\0')
			{
				pop3->body = TRUE;
				plugin->helper->message_set_body(
						pop3->message->message, NULL, 0,
						0);
				return 0;
			}
			if(pop3->body)
			{
				plugin->helper->message_set_body(
						pop3->message->message, answer,
						strlen(answer), 1);
				plugin->helper->message_set_body(
						pop3->message->message, "\r\n",
						2, 1);
			}
			else
				plugin->helper->message_set_header(
						pop3->message->message, answer);
			return 0;
		case P3C_TRANSACTION_STAT:
			if(sscanf(answer, "+OK %u %u", &u, &v) != 2)
				return -1;
			pop3->context = P3C_TRANSACTION_LIST;
			return _pop3_command(plugin, "LIST");
	}
	return -1;
}


/* pop3_message_get */
static AccountMessage * _pop3_message_get(AccountPlugin * plugin,
		AccountFolder * folder, unsigned int id)
{
	size_t i;

	for(i = 0; i < folder->messages_cnt; i++)
		if(folder->messages[i]->id == id)
			break;
	if(i != folder->messages_cnt)
		return folder->messages[i];
	return _pop3_message_new(plugin, folder, id);
}


/* pop3_message_new */
static AccountMessage * _pop3_message_new(AccountPlugin * plugin,
		AccountFolder * folder, unsigned int id)
{
	AccountMessage * message;
	AccountMessage ** p;

	if((p = realloc(folder->messages, sizeof(*p)
					* (folder->messages_cnt + 1))) == NULL)
		return NULL;
	folder->messages = p;
	if((message = object_new(sizeof(*message))) == NULL)
		return NULL;
	message->message = plugin->helper->message_new(plugin->helper->account,
			folder->folder, message);
	message->id = id;
	if(message->message == NULL)
	{
		_pop3_message_delete(plugin, message);
		return NULL;
	}
	folder->messages[folder->messages_cnt++] = message;
	return message;
}


/* pop3_message_delete */
static void _pop3_message_delete(AccountPlugin * plugin,
		AccountMessage * message)
{
	if(message->message != NULL)
		plugin->helper->message_delete(message->message);
	object_delete(message);
}


/* callbacks */
/* on_idle */
static gboolean _idle_connect(AccountPlugin * plugin);
static gboolean _idle_channel(AccountPlugin * plugin);

static gboolean _on_idle(gpointer data)
{
	gboolean ret = FALSE;
	AccountPlugin * plugin = data;
	POP3 * pop3 = plugin->priv;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(pop3->fd < 0)
		ret = _idle_connect(plugin);
	else if(pop3->channel == NULL)
		ret = _idle_channel(plugin);
	if(ret == FALSE)
		pop3->source = 0;
	return ret;
}

static gboolean _idle_connect(AccountPlugin * plugin)
{
	POP3 * pop3 = plugin->priv;
	char const * p;
	struct hostent * he;
	unsigned short port;
	struct sockaddr_in sa;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	/* FIXME report errors */
	if((p = plugin->config[2].value) == NULL)
		return FALSE;
	if((he = gethostbyname(p)) == NULL)
	{
		plugin->helper->error(NULL, hstrerror(h_errno), 1);
		return FALSE;
	}
	if((p = plugin->config[3].value) == NULL)
		return FALSE;
	port = (unsigned long)p;
	if((pop3->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		plugin->helper->error(NULL, strerror(errno), 1);
		return FALSE;
	}
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = *((uint32_t*)he->h_addr_list[0]);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() connecting to %s:%u\n", __func__,
			inet_ntoa(sa.sin_addr), port);
#endif
	if(connect(pop3->fd, (struct sockaddr *)&sa, sizeof(sa)) != 0)
	{
		plugin->helper->error(NULL, strerror(errno), 1);
		close(pop3->fd);
		pop3->fd = -1;
		return FALSE;
	}
	return TRUE;
}

static gboolean _idle_channel(AccountPlugin * plugin)
{
	POP3 * pop3 = plugin->priv;
	GError * error = NULL;

	pop3->channel = g_io_channel_unix_new(pop3->fd);
	g_io_channel_set_encoding(pop3->channel, NULL, &error);
	g_io_channel_set_buffered(pop3->channel, FALSE);
	pop3->rd_source = g_io_add_watch(pop3->channel, G_IO_IN,
			_on_watch_can_read, plugin);
	return TRUE;
}


/* on_watch_can_read */
static gboolean _on_watch_can_read(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	AccountPlugin * plugin = data;
	POP3 * pop3 = plugin->priv;
	char * p;
	gsize cnt = 0;
	GError * error = NULL;
	GIOStatus status;

	if(condition != G_IO_IN || source != pop3->channel)
		return FALSE; /* should not happen */
	if((p = realloc(pop3->rd_buf, pop3->rd_buf_cnt + 256)) == NULL)
		return TRUE; /* XXX retries immediately (delay?) */
	pop3->rd_buf = p;
	status = g_io_channel_read_chars(source,
			&pop3->rd_buf[pop3->rd_buf_cnt], 256, &cnt, &error);
#ifdef DEBUG
	fprintf(stderr, "%s", "DEBUG: POP3 SERVER: ");
	fwrite(&pop3->rd_buf[pop3->rd_buf_cnt], sizeof(*p), cnt, stderr);
#endif
	pop3->rd_buf_cnt += cnt;
	switch(status)
	{
		case G_IO_STATUS_NORMAL:
			break;
		case G_IO_STATUS_ERROR:
			plugin->helper->error(NULL, error->message, 1);
		case G_IO_STATUS_EOF:
		default: /* XXX find a way to recover */
			pop3->rd_source = 0;
			return FALSE;
	}
	_pop3_parse(plugin);
	return TRUE;
}


/* on_watch_can_write */
static gboolean _on_watch_can_write(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	AccountPlugin * plugin = data;
	POP3 * pop3 = plugin->priv;
	gsize cnt;
	GError * error = NULL;
	GIOStatus status;
	char * p;

	if(condition != G_IO_OUT || source != pop3->channel)
		return FALSE; /* should not happen */
	status = g_io_channel_write_chars(source, pop3->wr_buf,
			pop3->wr_buf_cnt, &cnt, &error);
#ifdef DEBUG
	fprintf(stderr, "%s", "DEBUG: POP3 CLIENT: ");
	fwrite(pop3->wr_buf, sizeof(*p), cnt, stderr);
#endif
	if(cnt != 0)
	{
		pop3->wr_buf_cnt -= cnt;
		memmove(pop3->wr_buf, &pop3->wr_buf[cnt], pop3->wr_buf_cnt);
		if((p = realloc(pop3->wr_buf, pop3->wr_buf_cnt)) != NULL)
			pop3->wr_buf = p; /* we can ignore errors... */
		else if(pop3->wr_buf_cnt == 0)
			pop3->wr_buf = NULL; /* ...except when it's not one */
	}
	switch(status)
	{
		case G_IO_STATUS_NORMAL:
			break;
		case G_IO_STATUS_ERROR:
			plugin->helper->error(NULL, error->message, 1);
		case G_IO_STATUS_EOF:
		default: /* XXX find a way to recover */
			pop3->wr_source = 0;
			return FALSE;
	}
	if(pop3->wr_buf_cnt > 0)
		return TRUE;
	pop3->wr_source = 0;
	return FALSE;
}
