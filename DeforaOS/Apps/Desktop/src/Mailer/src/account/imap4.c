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
 * - support multiple connections? */



#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <glib.h>
#include <System.h>
#include "Mailer.h"


/* IMAP4 */
/* private */
/* types */
struct _AccountFolder
{
	Folder * folder;

	char * name;

	AccountMessage ** messages;
	size_t messages_cnt;

	AccountFolder ** folders;
	size_t folders_cnt;
};

struct _AccountMessage
{
	Message * message;

	unsigned int id;
};

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
	I4C_FETCH,
	I4C_LIST,
	I4C_LOGIN,
	I4C_NOOP,
	I4C_SELECT
} IMAP4Context;

typedef enum _IMAP4FetchStatus
{
	I4FS_ID = 0,
	I4FS_HEADERS,
	I4FS_BODY
} IMAP4FetchStatus;

typedef struct _IMAP4Command
{
	uint16_t id;
	IMAP4CommandStatus status;
	IMAP4Context context;
	char * buf;
	size_t buf_cnt;

	union
	{
		struct
		{
			AccountFolder * folder;
			AccountMessage * message;
			unsigned int id;
			IMAP4FetchStatus status;
		} fetch;

		struct
		{
			AccountFolder * folder;
		} select;
	} data;
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

	AccountFolder folders;
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
static int _imap4_refresh(AccountPlugin * plugin, AccountFolder * folder,
		AccountMessage * message);

/* useful */
static IMAP4Command * _imap4_command(AccountPlugin * plugin,
		IMAP4Context context, char const * command);
static int _imap4_parse(AccountPlugin * plugin);

static AccountFolder * _imap4_folder_new(AccountPlugin * plugin,
		AccountFolder * parent, char const * name);
static void _imap4_folder_delete(AccountPlugin * plugin,
		AccountFolder * folder);
static AccountFolder * _imap4_folder_get_folder(AccountPlugin * plugin,
		AccountFolder * folder, char const * name);
static AccountMessage * _imap4_folder_get_message(AccountPlugin * plugin,
		AccountFolder * folder, unsigned int id);

static AccountMessage * _imap4_message_new(AccountPlugin * plugin,
		AccountFolder * folder, unsigned int id);
static void _imap4_message_delete(AccountPlugin * plugin,
		AccountMessage * message);

/* callbacks */
static gboolean _on_idle(gpointer data);
static gboolean _on_noop(gpointer data);
static gboolean _on_reset(gpointer data);
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
	_imap4_refresh,
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
	imap4->source = g_idle_add(_on_idle, plugin);
	return 0;
}


/* imap4_destroy */
static int _imap4_destroy(AccountPlugin * plugin)
{
	IMAP4 * imap4 = plugin->priv;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(imap4 == NULL) /* XXX _imap4_destroy() may be called uninitialized */
		return 0;
	if(imap4->rd_source != 0)
		g_source_remove(imap4->rd_source);
	if(imap4->wr_source != 0)
		g_source_remove(imap4->wr_source);
	if(imap4->source != 0)
		g_source_remove(imap4->source);
	if(imap4->channel != NULL)
	{
		g_io_channel_shutdown(imap4->channel, TRUE, NULL);
		g_io_channel_unref(imap4->channel);
		imap4->fd = -1;
	}
	for(i = 0; i < imap4->queue_cnt; i++)
		free(imap4->queue[i].buf);
	free(imap4->queue);
	if(imap4->fd >= 0)
		close(imap4->fd);
#if 0 /* XXX do not free() */
	_imap4_folder_delete(plugin, &imap4->folders);
#endif
	free(imap4);
	return 0;
}


/* imap4_refresh */
static int _imap4_refresh(AccountPlugin * plugin, AccountFolder * folder,
		AccountMessage * message)
{
	char buf[32];
	IMAP4Command * cmd;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %u\n", __func__, message->id);
#endif
	snprintf(buf, sizeof(buf), "FETCH %u BODY[]", message->id);
	if((cmd = _imap4_command(plugin, I4C_FETCH, buf)) == NULL)
		return -1;
	cmd->data.fetch.folder = folder;
	cmd->data.fetch.message = message;
	cmd->data.fetch.id = message->id;
	cmd->data.fetch.status = I4FS_ID;
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
	/* abort if the command is invalid */
	if(command == NULL || (len = strlen(command) + 6) == 0)
		return NULL;
	/* abort if there is no active connection */
	if(imap4->channel == NULL)
		return NULL;
	/* queue the command */
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
	memset(&p->data, 0, sizeof(p->data));
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
static int _context_fetch(AccountPlugin * plugin, char const * answer);
static int _context_list(AccountPlugin * plugin, char const * answer);
static int _context_select(AccountPlugin * plugin, char const * answer);

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
	IMAP4 * imap4 = plugin->priv;
	IMAP4Command * cmd = &imap4->queue[0];
	char const * p;
	char const * q;
	gchar * r;
	char const list[] = "LIST \"*\" \"*\"";

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\") %u, %u\n", __func__, answer,
			cmd->context, cmd->status);
#endif
	switch(cmd->context)
	{
		case I4C_FETCH:
			return _context_fetch(plugin, answer);
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
			return _context_list(plugin, answer);
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
			return _context_select(plugin, answer);
	}
	return ret;
}

static int _context_fetch(AccountPlugin * plugin, char const * answer)
{
	AccountPluginHelper * helper = plugin->helper;
	IMAP4 * imap4 = plugin->priv;
	IMAP4Command * cmd = &imap4->queue[0];
	AccountFolder * folder = cmd->data.fetch.folder;
	AccountMessage * message = cmd->data.fetch.message;
	unsigned int id = cmd->data.fetch.id;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\")\n", __func__, (void *)folder,
			answer);
#endif
	if(cmd->status == I4CS_PARSING)
	{
		cmd->status = I4CS_OK;
		return 0;
	}
	switch(cmd->data.fetch.status)
	{
		case I4FS_BODY:
#if 1 /* FIXME dirty hack for now */
			if(strcmp(answer, ")") == 0)
			{
				cmd->data.fetch.status = I4FS_ID;
				return 0;
			}
#endif
			helper->message_set_body(message->message, answer,
					strlen(answer), 1);
			helper->message_set_body(message->message, "\r\n", 2,
					1);
			return 0;
		case I4FS_ID:
			id = strtol(answer, &p, 10);
			if(answer[0] == '\0' || *p != ' ')
				return 0;
			if(cmd->data.fetch.id != id)
				cmd->data.fetch.id = id;
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() %u\n", __func__, id);
#endif
			answer = p;
			if(strncmp(answer, " FETCH ", 7) != 0)
				return 0;
			if((message = _imap4_folder_get_message(plugin, folder,
							id)) != NULL)
			{
				cmd->data.fetch.status = I4FS_HEADERS;
				cmd->data.fetch.message = message;
			}
			return (message != NULL) ? 0 : -1;
		case I4FS_HEADERS:
			if(strcmp(answer, "") == 0)
			{
				cmd->data.fetch.status = I4FS_BODY;
				helper->message_set_body(message->message, NULL,
						0, 0);
			}
			else
				helper->message_set_header(message->message,
						answer);
			return 0;
	}
	return -1;
}

static int _context_list(AccountPlugin * plugin, char const * answer)
{
	IMAP4 * imap4 = plugin->priv;
	IMAP4Command * cmd = &imap4->queue[0];
	AccountFolder * folder;
	char const * p;
	gchar * q;
	char buf[32];

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
	if(*p == '\"' && sscanf(++p, "%31[^\"]", buf) == 1
			&& (folder = _imap4_folder_get_folder(plugin,
					&imap4->folders, buf)) != NULL)
	{
		buf[31] = '\0';
		q = g_strdup_printf("%s \"%s\"", "SELECT", buf);
		if((cmd = _imap4_command(plugin, I4C_SELECT, q)) != NULL)
			cmd->data.select.folder = folder;
		g_free(q);
	}
	return (cmd != NULL) ? 0 : -1;
}

static int _context_select(AccountPlugin * plugin, char const * answer)
{
	IMAP4 * imap4 = plugin->priv;
	IMAP4Command * cmd = &imap4->queue[0];
	AccountFolder * folder;
	char const fetch[] = "FETCH 1:* BODY[HEADER]";

	if(cmd->status != I4CS_PARSING)
		return 0;
	cmd->status = I4CS_OK;
	if((folder = cmd->data.select.folder) == NULL)
		return 0; /* XXX really is an error */
	if((cmd = _imap4_command(plugin, I4C_FETCH, fetch)) != NULL)
	{
		cmd->data.fetch.folder = folder;
		cmd->data.fetch.message = NULL;
		cmd->data.fetch.id = 0;
		cmd->data.fetch.status = I4FS_ID;
	}
	return (cmd != NULL) ? 0 : -1;
}


/* imap4_folder_new */
static AccountFolder * _imap4_folder_new(AccountPlugin * plugin,
		AccountFolder * parent, char const * name)
{
	AccountPluginHelper * helper = plugin->helper;
	AccountFolder * folder;
	AccountFolder ** p;
	IMAP4 * imap4 = plugin->priv;
	FolderType type = FT_FOLDER;
	struct
	{
		char const * name;
		FolderType type;
	} name_type[] =
	{
		{ "Inbox",	FT_INBOX	},
		{ "Drafts",	FT_DRAFTS	},
		{ "Trash",	FT_TRASH	},
		{ "Sent",	FT_SENT		},
		{ NULL,		0		}
	};
	size_t i;

	if((p = realloc(parent->folders, sizeof(*p) * (parent->folders_cnt
						+ 1))) == NULL)
		return NULL;
	parent->folders = p;
	if((folder = object_new(sizeof(*folder))) == NULL)
		return NULL;
	if(parent == &imap4->folders)
		for(i = 0; name_type[i].name != NULL; i++)
			if(strcasecmp(name_type[i].name, name) == 0)
			{
				type = name_type[i].type;
				break;
			}
	folder->folder = helper->folder_new(helper->account, folder, NULL, type,
			name);
	folder->name = strdup(name);
	folder->messages = NULL;
	folder->messages_cnt = 0;
	folder->folders = NULL;
	folder->folders_cnt = 0;
	if(folder->folder == NULL || folder->name == NULL)
	{
		_imap4_folder_delete(plugin, folder);
		return NULL;
	}
	parent->folders[parent->folders_cnt++] = folder;
	return folder;
}


/* imap4_folder_delete */
static void _imap4_folder_delete(AccountPlugin * plugin, AccountFolder * folder)
{
	size_t i;

	if(folder->folder != NULL)
		plugin->helper->folder_delete(folder->folder);
	free(folder->name);
	for(i = 0; i < folder->messages_cnt; i++)
		_imap4_message_delete(plugin, folder->messages[i]);
	free(folder->messages);
	for(i = 0; i < folder->folders_cnt; i++)
		_imap4_folder_delete(plugin, folder->folders[i]);
	free(folder->folders);
	object_delete(folder);
}


/* imap4_folder_get_folder */
static AccountFolder * _imap4_folder_get_folder(AccountPlugin * plugin,
		AccountFolder * folder, char const * name)
{
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", \"%s\")\n", __func__, folder->name,
			name);
#endif
	for(i = 0; i < folder->folders_cnt; i++)
		if(strcmp(folder->folders[i]->name, name) == 0)
			return folder->folders[i];
	return _imap4_folder_new(plugin, folder, name);
}


/* imap4_folder_get_message */
static AccountMessage * _imap4_folder_get_message(AccountPlugin * plugin,
		AccountFolder * folder, unsigned int id)
{
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", %u)\n", __func__, folder->name, id);
#endif
	for(i = 0; i < folder->messages_cnt; i++)
		if(folder->messages[i]->id == id)
			return folder->messages[i];
	return _imap4_message_new(plugin, folder, id);
}


/* imap4_message_new */
static AccountMessage * _imap4_message_new(AccountPlugin * plugin,
		AccountFolder * folder, unsigned int id)
{
	AccountPluginHelper * helper = plugin->helper;
	AccountMessage * message;
	AccountMessage ** p;

	if((p = realloc(folder->messages, sizeof(*p)
					* (folder->messages_cnt + 1))) == NULL)
		return NULL;
	folder->messages = p;
	if((message = object_new(sizeof(*message))) == NULL)
		return NULL;
	message->id = id;
	if((message->message = helper->message_new(helper->account,
					folder->folder, message)) == NULL)
	{
		_imap4_message_delete(plugin, message);
		return NULL;
	}
	folder->messages[folder->messages_cnt++] = message;
	return message;
}


/* imap4_message_delete */
static void _imap4_message_delete(AccountPlugin * plugin,
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
	/* prepare queue */
	if((imap4->queue = malloc(sizeof(*imap4->queue))) == NULL)
		return FALSE;
	imap4->queue[0].id = 0;
	imap4->queue[0].context = I4C_INIT;
	imap4->queue[0].status = I4CS_SENT;
	imap4->queue[0].buf = NULL;
	imap4->queue[0].buf_cnt = 0;
	imap4->queue_cnt = 1;
	imap4->queue_id = 1;
	/* setup channel */
	imap4->channel = g_io_channel_unix_new(imap4->fd);
	g_io_channel_set_encoding(imap4->channel, NULL, &error);
	g_io_channel_set_buffered(imap4->channel, FALSE);
	/* wait for the server's banner */
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


/* on_reset */
static gboolean _on_reset(gpointer data)
{
	AccountPlugin * plugin = data;
	IMAP4 * imap4 = plugin->priv;
	size_t i;

	/* FIXME merge with _imap4_destroy() */
	if(imap4->rd_source != 0)
		g_source_remove(imap4->rd_source);
	imap4->rd_source = 0;
	if(imap4->wr_source != 0)
		g_source_remove(imap4->wr_source);
	imap4->wr_source = 0;
	if(imap4->source != 0)
		g_source_remove(imap4->source);
	imap4->source = 0;
	if(imap4->channel != NULL)
	{
		g_io_channel_shutdown(imap4->channel, TRUE, NULL);
		g_io_channel_unref(imap4->channel);
		imap4->fd = -1;
	}
	imap4->channel = NULL;
	for(i = 0; i < imap4->queue_cnt; i++)
		free(imap4->queue[i].buf);
	free(imap4->queue);
	imap4->queue = NULL;
	imap4->queue_cnt = 0;
	if(imap4->fd >= 0)
		close(imap4->fd);
	imap4->source = g_timeout_add(3000, _on_idle, plugin);
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
		default:
			imap4->rd_source = g_idle_add(_on_reset, plugin);
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
		default:
			imap4->wr_source = g_idle_add(_on_reset, plugin);
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
