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
 * - no longer block on connect()
 * - recovery on errors
 * - really queue commands with callbacks
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

typedef enum _POP3CommandStatus
{
	P3CS_QUEUED = 0,
	P3CS_SENT,
	P3CS_ERROR,
	P3CS_PARSING,
	P3CS_OK
} POP3CommandStatus;

typedef enum _POP3Context
{
	P3C_INIT = 0,
	P3C_AUTHORIZATION_USER,
	P3C_AUTHORIZATION_PASS,
	P3C_NOOP,
	P3C_TRANSACTION_LIST,
	P3C_TRANSACTION_RETR,
	P3C_TRANSACTION_STAT,
	P3C_TRANSACTION_TOP
} POP3Context;

typedef struct _POP3Command
{
	POP3CommandStatus status;
	POP3Context context;
	char * buf;
	size_t buf_cnt;

	union
	{
		struct
		{
			unsigned int id;
			gboolean body;
			AccountMessage * message;
		} transaction_retr, transaction_top;
	} data;
} POP3Command;

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
	guint wr_source;

	POP3Command * queue;
	size_t queue_cnt;
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
/* plug-in */
static int _pop3_init(AccountPlugin * plugin);
static int _pop3_destroy(AccountPlugin * plugin);
static int _pop3_refresh(AccountPlugin * plugin, AccountFolder * folder,
		AccountMessage * message);

/* useful */
static POP3Command * _pop3_command(AccountPlugin * plugin, POP3Context context,
		char const * command);
static int _pop3_parse(AccountPlugin * plugin);

static AccountMessage * _pop3_message_get(AccountPlugin * plugin,
		AccountFolder * folder, unsigned int id);
static AccountMessage * _pop3_message_new(AccountPlugin * plugin,
		AccountFolder * folder, unsigned int id);
static void _pop3_message_delete(AccountPlugin * plugin,
		AccountMessage * message);

/* callbacks */
static gboolean _on_idle(gpointer data);
static gboolean _on_noop(gpointer data);
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
	if((pop3->queue = malloc(sizeof(*pop3->queue))) == NULL)
	{
		free(pop3);
		return -1;
	}
	plugin->priv = pop3;
	pop3->fd = -1;
	pop3->source = 0;
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
	pop3->wr_source = 0;
	pop3->queue[0].context = P3C_INIT;
	pop3->queue[0].status = P3CS_SENT;
	pop3->queue[0].buf = NULL;
	pop3->queue[0].buf_cnt = 0;
	pop3->queue_cnt = 1;
	pop3->source = g_idle_add(_on_idle, plugin);
	return 0;
}


/* pop3_destroy */
static int _pop3_destroy(AccountPlugin * plugin)
{
	POP3 * pop3 = plugin->priv;
	size_t i;

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
	if(pop3->source != 0)
		g_source_remove(pop3->source);
	if(pop3->channel != NULL)
	{
		g_io_channel_shutdown(pop3->channel, TRUE, NULL);
		g_io_channel_unref(pop3->channel);
		pop3->fd = -1;
	}
	for(i = 0; i < pop3->queue_cnt; i++)
		free(pop3->queue[i].buf);
	free(pop3->queue);
	if(pop3->fd >= 0)
		close(pop3->fd);
	free(pop3);
	return 0;
}


/* pop3_refresh */
static int _pop3_refresh(AccountPlugin * plugin, AccountFolder * folder,
		AccountMessage * message)
{
	char buf[32];
	POP3Command * cmd;

	snprintf(buf, sizeof(buf), "%s %u", "RETR", message->id);
	if((cmd = _pop3_command(plugin, P3C_TRANSACTION_RETR, buf)) == NULL)
		return -1;
	cmd->data.transaction_retr.id = message->id;
	return 0;
}


/* useful */
/* pop3_command */
static POP3Command * _pop3_command(AccountPlugin * plugin, POP3Context context,
		char const * command)
{
	POP3 * pop3 = plugin->priv;
	POP3Command * p;
	size_t len;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, command);
#endif
	if(command == NULL || (len = strlen(command)) == 0)
		return NULL;
	len += 2;
	if((p = realloc(pop3->queue, sizeof(*p) * (pop3->queue_cnt + 1)))
			== NULL)
		return NULL;
	pop3->queue = p;
	p = &pop3->queue[pop3->queue_cnt];
	p->context = context;
	p->status = P3CS_QUEUED;
	if((p->buf = malloc(len + 1)) == NULL)
		return NULL;
	p->buf_cnt = snprintf(p->buf, len + 1, "%s\r\n", command);
	memset(&p->data, 0, sizeof(p->data));
	if(pop3->queue_cnt++ == 0)
	{
		if(pop3->source != 0)
		{
			/* cancel the pending NOOP operation */
			g_source_remove(pop3->source);
			pop3->source = 0;
		}
		pop3->wr_source = g_io_add_watch(pop3->channel, G_IO_OUT,
				_on_watch_can_write, plugin);
	}
	return p;
}


/* pop3_parse */
static int _parse_context(AccountPlugin * plugin, char const * answer);
static int _parse_context_transaction_retr(AccountPlugin * plugin,
		char const * answer);

static int _pop3_parse(AccountPlugin * plugin)
{
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
		if(pop3->queue_cnt == 0)
			continue;
		if(pop3->queue[0].status == P3CS_SENT
				&& strncmp("-ERR", &pop3->rd_buf[j], 4) == 0)
		{
			pop3->queue[0].status = P3CS_ERROR;
			plugin->helper->error(plugin->helper->account,
					&pop3->rd_buf[j + 4], 1);
		}
		else if(pop3->queue[0].status == P3CS_SENT
				&& strncmp("+OK", &pop3->rd_buf[j], 3) == 0)
			pop3->queue[0].status = P3CS_PARSING;
		if(_parse_context(plugin, &pop3->rd_buf[j]) != 0)
			pop3->queue[0].status = P3CS_ERROR;
	}
	if(j != 0)
	{
		pop3->rd_buf_cnt -= j;
		memmove(pop3->rd_buf, &pop3->rd_buf[j], pop3->rd_buf_cnt);
	}
	return (pop3->queue[0].status != P3CS_ERROR) ? 0 : -1;
}

static int _parse_context(AccountPlugin * plugin, char const * answer)
{
	int ret = -1;
	POP3 * pop3 = plugin->priv;
	POP3Command * cmd;
	char const * p;
	char * q;
	unsigned int u;
	unsigned int v;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\") %u, %u\n", __func__, answer,
			pop3->queue[0].context, pop3->queue[0].status);
#endif
	switch(pop3->queue[0].context)
	{
		case P3C_INIT:
			if(pop3->queue[0].status != P3CS_PARSING)
				return 0;
			pop3->queue[0].status = P3CS_OK;
			if((p = plugin->config[0].value) == NULL)
				return -1;
			q = g_strdup_printf("%s %s", "USER", p);
			cmd = _pop3_command(plugin, P3C_AUTHORIZATION_USER, q);
			g_free(q);
			return (cmd != NULL) ? 0 : -1;
		case P3C_AUTHORIZATION_USER:
			if(pop3->queue[0].status != P3CS_PARSING)
				return 0;
			pop3->queue[0].status = P3CS_OK;
			if((p = plugin->config[1].value) == NULL)
				p = ""; /* assumes an empty password */
			q = g_strdup_printf("%s %s", "PASS", p);
			cmd = _pop3_command(plugin, P3C_AUTHORIZATION_PASS, q);
			g_free(q);
			return (cmd != NULL) ? 0 : -1;
		case P3C_AUTHORIZATION_PASS:
			if(pop3->queue[0].status != P3CS_PARSING)
				return 0;
			pop3->queue[0].status = P3CS_OK;
			return (_pop3_command(plugin, P3C_TRANSACTION_STAT,
						"STAT") != NULL) ? 0 : -1;
		case P3C_NOOP:
			pop3->queue[0].status = P3CS_OK;
			if(strncmp(answer, "+OK", 3) == 0)
				return 0;
			return -1;
		case P3C_TRANSACTION_LIST:
			if(pop3->queue[0].status != P3CS_PARSING)
				return 0;
			if(strncmp(answer, "+OK", 3) == 0)
				return 0;
			if(strcmp(answer, ".") == 0)
			{
				pop3->queue[0].status = P3CS_OK;
				return 0;
			}
			if(sscanf(answer, "%u %u", &u, &v) != 2)
				return -1;
			/* FIXME may not be supported by the server */
			q = g_strdup_printf("%s %u 0", "TOP", u);
			cmd = _pop3_command(plugin, P3C_TRANSACTION_TOP, q);
			free(q);
			cmd->data.transaction_top.id = u;
			return (cmd != NULL) ? 0 : -1;
		case P3C_TRANSACTION_RETR:
		case P3C_TRANSACTION_TOP: /* same as RETR without the body */
			return _parse_context_transaction_retr(plugin, answer);
		case P3C_TRANSACTION_STAT:
			if(pop3->queue[0].status != P3CS_PARSING)
				return 0;
			if(sscanf(answer, "+OK %u %u", &u, &v) != 2)
				return -1;
			pop3->queue[0].status = P3CS_OK;
			return (_pop3_command(plugin, P3C_TRANSACTION_LIST,
						"LIST") != NULL) ? 0 : -1;
	}
	return ret;
}

static int _parse_context_transaction_retr(AccountPlugin * plugin,
		char const * answer)
{
	POP3 * pop3 = plugin->priv;
	POP3Command * cmd = &pop3->queue[0];

	if(cmd->status != P3CS_PARSING)
		return 0;
	if(cmd->data.transaction_retr.message == NULL
			&& strncmp(answer, "+OK", 3) == 0)
	{
		cmd->data.transaction_retr.body = FALSE;
		cmd->data.transaction_retr.message = _pop3_message_get(plugin,
					&pop3->inbox,
					cmd->data.transaction_retr.id);
		return 0;
	}
	if(strcmp(answer, ".") == 0)
	{
		cmd->status = P3CS_OK;
		return 0;
	}
	if(answer[0] == '\0')
	{
		cmd->data.transaction_retr.body = TRUE;
		plugin->helper->message_set_body(
				cmd->data.transaction_retr.message->message,
				NULL, 0, 0);
		return 0;
	}
	if(cmd->data.transaction_retr.body)
	{
		plugin->helper->message_set_body(
				cmd->data.transaction_retr.message->message,
				answer, strlen(answer), 1);
		plugin->helper->message_set_body(
				cmd->data.transaction_retr.message->message,
				"\r\n", 2, 1);
	}
	else
		plugin->helper->message_set_header(
				cmd->data.transaction_retr.message->message,
				answer);
	return 0;
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

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	pop3->channel = g_io_channel_unix_new(pop3->fd);
	g_io_channel_set_encoding(pop3->channel, NULL, &error);
	g_io_channel_set_buffered(pop3->channel, FALSE);
	pop3->rd_source = g_io_add_watch(pop3->channel, G_IO_IN,
			_on_watch_can_read, plugin);
	return TRUE;
}


/* on_noop */
static gboolean _on_noop(gpointer data)
{
	AccountPlugin * plugin = data;
	POP3 * pop3 = plugin->priv;

	_pop3_command(plugin, P3C_NOOP, "NOOP");
	pop3->source = 0;
	return FALSE;
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
	POP3Command * cmd;

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
	if(_pop3_parse(plugin) != 0 || pop3->queue_cnt == 0)
	{
		pop3->rd_source = 0;
		return FALSE;
	}
	cmd = &pop3->queue[0];
	if(cmd->buf_cnt == 0)
	{
		if(cmd->status == P3CS_SENT || cmd->status == P3CS_PARSING)
			return TRUE;
		else if(cmd->status == P3CS_OK)
			memmove(cmd, &pop3->queue[1], sizeof(*cmd)
					* --pop3->queue_cnt);
	}
	pop3->rd_source = 0;
	if(pop3->queue_cnt == 0)
		pop3->source = g_timeout_add(30000, _on_noop, plugin);
	else
		pop3->wr_source = g_io_add_watch(pop3->channel, G_IO_OUT,
				_on_watch_can_write, plugin);
	return FALSE;
}


/* on_watch_can_write */
static gboolean _on_watch_can_write(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	AccountPlugin * plugin = data;
	POP3 * pop3 = plugin->priv;
	POP3Command * cmd = &pop3->queue[0];
	gsize cnt = 0;
	GError * error = NULL;
	GIOStatus status;
	char * p;

	if(condition != G_IO_OUT || source != pop3->channel
			|| pop3->queue_cnt == 0 || cmd->buf_cnt == 0)
		return FALSE; /* should not happen */
	status = g_io_channel_write_chars(source, cmd->buf, cmd->buf_cnt, &cnt,
			&error);
#ifdef DEBUG
	fprintf(stderr, "%s", "DEBUG: POP3 CLIENT: ");
	fwrite(cmd->buf, sizeof(*p), cnt, stderr);
#endif
	if(cnt != 0)
	{
		cmd->buf_cnt -= cnt;
		memmove(cmd->buf, &cmd->buf[cnt], cmd->buf_cnt);
		if((p = realloc(cmd->buf, cmd->buf_cnt)) != NULL)
			cmd->buf = p; /* we can ignore errors... */
		else if(cmd->buf_cnt == 0)
			cmd->buf = NULL; /* ...except when it's not one */
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
	if(cmd->buf_cnt > 0)
		return TRUE;
	cmd->status = P3CS_SENT;
	pop3->wr_source = 0;
	if(pop3->rd_source == 0)
		pop3->rd_source = g_io_add_watch(pop3->channel, G_IO_IN,
				_on_watch_can_read, plugin);
	return FALSE;
}
