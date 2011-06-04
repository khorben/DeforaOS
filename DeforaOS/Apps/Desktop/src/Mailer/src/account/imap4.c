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
 * - support multiple connections? */



#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <glib.h>
#include "Mailer.h"


/* IMAP4 */
/* private */
/* types */
typedef enum _IMAP4CommandStatus
{
	I4CS_QUEUED = 0,
	I4CS_SENT,
	I4CS_ERROR,
	I4CS_PARSING,
	I4CS_OK
} IMAP4CommandStatus;

typedef enum _IMAP4Context
{
	I4C_INIT = 0,
	I4C_LIST,
	I4C_LOGIN,
	I4C_NOOP,
	I4C_SELECT
} IMAP4Context;

typedef struct _IMAP4Command
{
	uint16_t id;
	IMAP4CommandStatus status;
	IMAP4Context context;
	char * buf;
	size_t buf_cnt;
} IMAP4Command;

typedef struct _IMAP4
{
	int fd;
	guint source;

	GIOChannel * channel;
	char * rd_buf;
	size_t rd_buf_cnt;
	guint rd_source;
	guint wr_source;

	IMAP4Command * queue;
	size_t queue_cnt;
	uint16_t queue_id;
} IMAP4;


/* variables */
static char const _imap4_type[] = "IMAP4";
static char const _imap4_name[] = "IMAP4 server";

AccountConfig _imap4_config[] =
{
	{ "username",	"Username",		ACT_STRING,	NULL	},
	{ "password",	"Password",		ACT_PASSWORD,	NULL	},
	{ "hostname",	"Server hostname",	ACT_STRING,	NULL	},
	{ "port",	"Server port",		ACT_UINT16,	994	},
#if 0 /* FIXME SSL is not supported yet */
	{ "ssl",	"Use SSL",		ACT_BOOLEAN,	1	},
#endif
	{ "sent",	"Sent mails folder",	ACT_NONE,	NULL	},
	{ "draft",	"Draft mails folder",	ACT_NONE,	NULL	},
	{ NULL,		NULL,			ACT_NONE,	NULL	}
};


/* prototypes */
/* plug-in */
static int _imap4_init(AccountPlugin * plugin);
static int _imap4_destroy(AccountPlugin * plugin);

/* useful */
static IMAP4Command * _imap4_command(AccountPlugin * plugin,
		IMAP4Context context, char const * command);
static int _imap4_parse(AccountPlugin * plugin);

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
	_imap4_type,
	_imap4_name,
	NULL,
	_imap4_config,
	_imap4_init,
	_imap4_destroy,
	NULL,
	NULL,
	NULL
};


/* private */
/* imap4_init */
static int _imap4_init(AccountPlugin * plugin)
{
	IMAP4 * imap4;

	if((imap4 = malloc(sizeof(*imap4))) == NULL)
		return -1;
	memset(imap4, 0, sizeof(*imap4));
	plugin->priv = imap4;
	imap4->fd = -1;
	if((imap4->queue = malloc(sizeof(*imap4->queue))) == NULL)
	{
		free(imap4);
		return -1;
	}
	imap4->queue[0].id = 0;
	imap4->queue[0].context = I4C_INIT;
	imap4->queue[0].status = I4CS_SENT;
	imap4->queue[0].buf = NULL;
	imap4->queue[0].buf_cnt = 0;
	imap4->queue_cnt = 1;
	imap4->queue_id = 1;
	imap4->source = g_idle_add(_on_idle, plugin);
	return 0;
}


/* imap4_destroy */
static int _imap4_destroy(AccountPlugin * plugin)
{
	IMAP4 * imap = plugin->priv;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	/* FIXME implement */
	free(imap);
	return 0;
}


/* useful */
/* imap4_command */
static IMAP4Command * _imap4_command(AccountPlugin * plugin,
		IMAP4Context context, char const * command)
{
	IMAP4 * imap4 = plugin->priv;
	IMAP4Command * p;
	size_t len;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, command);
#endif
	if(command == NULL || (len = strlen(command) + 6) == 0)
		return NULL;
	len += 2;
	if((p = realloc(imap4->queue, sizeof(*p) * (imap4->queue_cnt + 1)))
			== NULL)
		return NULL;
	imap4->queue = p;
	p = &imap4->queue[imap4->queue_cnt];
	p->id = imap4->queue_id++;
	p->context = context;
	p->status = I4CS_QUEUED;
	if((p->buf = malloc(len + 1)) == NULL)
		return NULL;
	p->buf_cnt = snprintf(p->buf, len + 1, "a%04x %s\r\n", p->id, command);
#if 0 /* XXX later */
	memset(&p->data, 0, sizeof(p->data));
#endif
	if(imap4->queue_cnt++ == 0)
	{
		if(imap4->source != 0)
		{
			/* cancel the pending NOOP operation */
			g_source_remove(imap4->source);
			imap4->source = 0;
		}
		imap4->wr_source = g_io_add_watch(imap4->channel, G_IO_OUT,
				_on_watch_can_write, plugin);
	}
	return p;
}


/* imap4_parse */
static int _parse_context(AccountPlugin * plugin, char const * answer);

static int _imap4_parse(AccountPlugin * plugin)
{
	AccountPluginHelper * helper = plugin->helper;
	IMAP4 * imap4 = plugin->priv;
	size_t i;
	size_t j;
	IMAP4Command * cmd;
	char buf[8];

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	for(i = 0, j = 0;; j = ++i)
	{
		for(; i < imap4->rd_buf_cnt; i++)
			if(imap4->rd_buf[i] == '\r' && i + 1 < imap4->rd_buf_cnt
					&& imap4->rd_buf[++i] == '\n')
				break;
		if(i == imap4->rd_buf_cnt)
			break;
		if(imap4->queue_cnt == 0)
			continue;
		imap4->rd_buf[i - 1] = '\0';
		cmd = &imap4->queue[0];
		if(cmd->status == I4CS_SENT)
		{
			snprintf(buf, sizeof(buf), "a%04x ", cmd->id);
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() expecting \"%s\"\n",
					__func__, buf);
#endif
			if(strncmp(&imap4->rd_buf[j], "* ", 2) == 0)
				j += 2;
			else if(strncmp(&imap4->rd_buf[j], buf, 6) == 0)
			{
				j += 6;
				cmd->status = I4CS_PARSING;
				if(strncmp("BAD ", &imap4->rd_buf[j], 4) == 0)
					helper->error(helper->account,
							&imap4->rd_buf[j + 4],
							1);
			}
			else
				/* FIXME untested code path */
				break;
		}
		if(_parse_context(plugin, &imap4->rd_buf[j]) != 0)
			cmd->status = I4CS_ERROR;
	}
	if(j != 0)
	{
		imap4->rd_buf_cnt -= j;
		memmove(imap4->rd_buf, &imap4->rd_buf[j], imap4->rd_buf_cnt);
	}
	if(imap4->queue == NULL)
		return 0;
	return (imap4->queue[0].status != I4CS_ERROR) ? 0 : -1;
}

static int _parse_context(AccountPlugin * plugin, char const * answer)
{
	int ret = -1;
	AccountPluginHelper * helper = plugin->helper;
	IMAP4 * imap4 = plugin->priv;
	IMAP4Command * cmd = &imap4->queue[0];
	char const * p;
	char const * q;
	gchar * r;
	char const list[] = "LIST \"*\" \"*\"";
	char buf[32];

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\") %u, %u\n", __func__, answer,
			cmd->context, cmd->status);
#endif
	switch(cmd->context)
	{
		case I4C_INIT:
			cmd->status = I4CS_OK;
			if((p = plugin->config[0].value) == NULL || *p == '\0')
				return -1;
			if((q = plugin->config[1].value) == NULL || *q == '\0')
				return -1;
			r = g_strdup_printf("%s %s %s", "LOGIN", p, q);
			cmd = _imap4_command(plugin, I4C_LOGIN, r);
			g_free(r);
			return (cmd != NULL) ? 0 : -1;
		case I4C_LIST:
			p = answer;
			if(strncmp("OK", p, 2) == 0)
			{
				cmd->status = I4CS_OK;
				return 0;
			}
			if(strncmp("LIST ", p, 5) != 0)
				return -1;
			p += 5;
			if(*p == '(')
				for(p++; *p != '\0' && *p++ != ')';);
			if(*p == ' ') /* skip spaces */
				for(p++; *p != '\0' && *p == ' '; p++);
			if(*p == '\"') /* skip reference */
				for(p++; *p != '\0' && *p++ != '\"';);
			if(*p == ' ') /* skip spaces */
				for(p++; *p != '\0' && *p == ' '; p++);
			if(*p == '\"' && sscanf(++p, "%31[^\"]", buf) == 1)
			{
				/* FIXME create metadata for this folder */
				helper->folder_new(helper->account, NULL,
						NULL, FT_INBOX, buf);
				buf[31] = '\0';
				r = g_strdup_printf("%s %s", "SELECT", buf);
				cmd = _imap4_command(plugin, I4C_SELECT, r);
				g_free(r);
			}
			return (cmd != NULL) ? 0 : -1;
		case I4C_LOGIN:
			if(cmd->status != I4CS_PARSING)
				return 0;
			cmd->status = I4CS_OK;
			return (_imap4_command(plugin, I4C_LIST, list) != NULL)
				? 0 : -1;
		case I4C_NOOP:
			if(cmd->status != I4CS_PARSING)
				return 0;
			cmd->status = I4CS_OK;
			return 0;
		case I4C_SELECT:
			if(cmd->status != I4CS_PARSING)
				return 0;
			/* FIXME implement */
			cmd->status = I4CS_OK;
			return 0;
	}
	return ret;
}


/* callbacks */
/* on_idle */
static gboolean _idle_connect(AccountPlugin * plugin);
static gboolean _idle_channel(AccountPlugin * plugin);

static gboolean _on_idle(gpointer data)
{
	gboolean ret = FALSE;
	AccountPlugin * plugin = data;
	IMAP4 * imap4 = plugin->priv;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(imap4->fd < 0)
		ret = _idle_connect(plugin);
	else if(imap4->channel == NULL)
		ret = _idle_channel(plugin);
	if(ret == FALSE)
		imap4->source = 0;
	return ret;
}

static gboolean _idle_connect(AccountPlugin * plugin)
{
	IMAP4 * imap4 = plugin->priv;
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
	if((imap4->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
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
	if(connect(imap4->fd, (struct sockaddr *)&sa, sizeof(sa)) != 0)
	{
		plugin->helper->error(NULL, strerror(errno), 1);
		close(imap4->fd);
		imap4->fd = -1;
		return FALSE;
	}
	return TRUE;
}

static gboolean _idle_channel(AccountPlugin * plugin)
{
	IMAP4 * imap4 = plugin->priv;
	GError * error = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	imap4->channel = g_io_channel_unix_new(imap4->fd);
	g_io_channel_set_encoding(imap4->channel, NULL, &error);
	g_io_channel_set_buffered(imap4->channel, FALSE);
	imap4->rd_source = g_io_add_watch(imap4->channel, G_IO_IN,
			_on_watch_can_read, plugin);
	return TRUE;
}


/* on_noop */
static gboolean _on_noop(gpointer data)
{
	AccountPlugin * plugin = data;
	IMAP4 * imap4 = plugin->priv;

	_imap4_command(plugin, I4C_NOOP, "NOOP");
	imap4->source = 0;
	return FALSE;
}


/* on_watch_can_read */
static gboolean _on_watch_can_read(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	AccountPlugin * plugin = data;
	IMAP4 * imap4 = plugin->priv;
	char * p;
	gsize cnt = 0;
	GError * error = NULL;
	GIOStatus status;
	IMAP4Command * cmd;

	if(condition != G_IO_IN || source != imap4->channel)
		return FALSE; /* should not happen */
	if((p = realloc(imap4->rd_buf, imap4->rd_buf_cnt + 256)) == NULL)
		return TRUE; /* XXX retries immediately (delay?) */
	imap4->rd_buf = p;
	status = g_io_channel_read_chars(source,
			&imap4->rd_buf[imap4->rd_buf_cnt], 256, &cnt, &error);
#ifdef DEBUG
	fprintf(stderr, "%s", "DEBUG: IMAP4 SERVER: ");
	fwrite(&imap4->rd_buf[imap4->rd_buf_cnt], sizeof(*p), cnt, stderr);
#endif
	imap4->rd_buf_cnt += cnt;
	switch(status)
	{
		case G_IO_STATUS_NORMAL:
			break;
		case G_IO_STATUS_ERROR:
			plugin->helper->error(NULL, error->message, 1);
		case G_IO_STATUS_EOF:
		default: /* XXX find a way to recover */
			imap4->rd_source = 0;
			return FALSE;
	}
	if(_imap4_parse(plugin) != 0 || imap4->queue_cnt == 0)
	{
		imap4->rd_source = 0;
		return FALSE;
	}
	cmd = &imap4->queue[0];
	if(cmd->buf_cnt == 0)
	{
		if(cmd->status == I4CS_SENT || cmd->status == I4CS_PARSING)
			return TRUE;
		else if(cmd->status == I4CS_OK)
			memmove(cmd, &imap4->queue[1], sizeof(*cmd)
					* --imap4->queue_cnt);
	}
	imap4->rd_source = 0;
	if(imap4->queue_cnt == 0)
		imap4->source = g_timeout_add(30000, _on_noop, plugin);
	else
		imap4->wr_source = g_io_add_watch(imap4->channel, G_IO_OUT,
				_on_watch_can_write, plugin);
	return FALSE;
}


/* on_watch_can_write */
static gboolean _on_watch_can_write(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	AccountPlugin * plugin = data;
	IMAP4 * imap4 = plugin->priv;
	IMAP4Command * cmd = &imap4->queue[0];
	gsize cnt = 0;
	GError * error = NULL;
	GIOStatus status;
	char * p;

	if(condition != G_IO_OUT || source != imap4->channel
			|| imap4->queue_cnt == 0 || cmd->buf_cnt == 0)
		return FALSE; /* should not happen */
	status = g_io_channel_write_chars(source, cmd->buf, cmd->buf_cnt, &cnt,
			&error);
#ifdef DEBUG
	fprintf(stderr, "%s", "DEBUG: IMAP4 CLIENT: ");
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
			imap4->wr_source = 0;
			return FALSE;
	}
	if(cmd->buf_cnt > 0)
		return TRUE;
	cmd->status = I4CS_SENT;
	imap4->wr_source = 0;
	if(imap4->rd_source == 0)
		imap4->rd_source = g_io_add_watch(imap4->channel, G_IO_IN,
				_on_watch_can_read, plugin);
	return FALSE;
}
