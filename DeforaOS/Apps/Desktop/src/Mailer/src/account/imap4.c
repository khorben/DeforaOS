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
 * - support multiple connections? */



#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
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

typedef enum _IMAP4ConfigValue
{
	I4CV_USERNAME = 0,
	I4CV_PASSWORD,
	I4CV_HOSTNAME,
	I4CV_PORT,
	I4CV_SSL,
	I4CV_PADDING0,
	I4CV_PREFIX
} IMAP4Config;
#define I4CV_LAST I4CV_PREFIX
#define I4CV_COUNT (I4CV_LAST + 1)

typedef enum _IMAP4Context
{
	I4C_INIT = 0,
	I4C_FETCH,
	I4C_LIST,
	I4C_LOGIN,
	I4C_NOOP,
	I4C_SELECT,
	I4C_STATUS
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
			AccountFolder * parent;
		} list;

		struct
		{
			AccountFolder * folder;
			AccountMessage * message;
		} select;

		struct
		{
			AccountFolder * folder;
		} status;
	} data;
} IMAP4Command;

typedef struct _IMAP4
{
	int fd;
	SSL_CTX * ssl_ctx;
	SSL * ssl;
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

AccountConfig _imap4_config[I4CV_COUNT + 1] =
{
	{ "username",	"Username",		ACT_STRING,	NULL	},
	{ "password",	"Password",		ACT_PASSWORD,	NULL	},
	{ "hostname",	"Server hostname",	ACT_STRING,	NULL	},
	{ "port",	"Server port",		ACT_UINT16,	143	},
	{ "ssl",	"Use SSL",		ACT_BOOLEAN,	0	},
#if 0 /* XXX not implemented yet */
	{ "sent",	"Sent mails folder",	ACT_NONE,	NULL	},
	{ "draft",	"Draft mails folder",	ACT_NONE,	NULL	},
#endif
	{ NULL,		NULL,			ACT_SEPARATOR,	NULL	},
	{ "prefix",	"Prefix",		ACT_STRING,	NULL	},
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
static gboolean _on_connect(gpointer data);
static gboolean _on_noop(gpointer data);
static gboolean _on_reset(gpointer data);
static gboolean _on_watch_can_connect(GIOChannel * source,
		GIOCondition condition, gpointer data);
static gboolean _on_watch_can_handshake(GIOChannel * source,
		GIOCondition condition, gpointer data);
static gboolean _on_watch_can_read(GIOChannel * source, GIOCondition condition,
		gpointer data);
static gboolean _on_watch_can_read_ssl(GIOChannel * source,
		GIOCondition condition, gpointer data);
static gboolean _on_watch_can_write(GIOChannel * source, GIOCondition condition,
		gpointer data);
static gboolean _on_watch_can_write_ssl(GIOChannel * source,
		GIOCondition condition, gpointer data);


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
	imap4->ssl_ctx = NULL;
	imap4->ssl = NULL;
	SSL_load_error_strings();
	SSL_library_init();
	imap4->source = g_idle_add(_on_connect, plugin);
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
	if(imap4->ssl_ctx != NULL)
		SSL_CTX_free(imap4->ssl_ctx);
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
	IMAP4Command * cmd;
	int len;
	char * buf;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %u\n", __func__, (message != NULL)
			? message->id : 0);
#endif
	if((len = snprintf(NULL, 0, "EXAMINE \"%s\"", folder->name)) < 0
			|| (buf = malloc(++len)) == NULL)
		return -1;
	snprintf(buf, len, "EXAMINE \"%s\"", folder->name);
	cmd = _imap4_command(plugin, I4C_SELECT, buf);
	free(buf);
	if(cmd == NULL)
		return -1;
	cmd->data.select.folder = folder;
	cmd->data.select.message = message;
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
				(imap4->ssl != NULL) ? _on_watch_can_write_ssl
				: _on_watch_can_write, plugin);
	}
	return p;
}


/* imap4_parse */
static int _parse_context(AccountPlugin * plugin, char const * answer);
static int _context_fetch(AccountPlugin * plugin, char const * answer);
static int _context_init(AccountPlugin * plugin);
static int _context_list(AccountPlugin * plugin, char const * answer);
static int _context_login(AccountPlugin * plugin, char const * answer);
static int _context_select(AccountPlugin * plugin);
static int _context_status(AccountPlugin * plugin, char const * answer);

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

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\") %u, %u\n", __func__, answer,
			cmd->context, cmd->status);
#endif
	switch(cmd->context)
	{
		case I4C_FETCH:
			return _context_fetch(plugin, answer);
		case I4C_INIT:
			return _context_init(plugin);
		case I4C_LIST:
			return _context_list(plugin, answer);
		case I4C_LOGIN:
			return _context_login(plugin, answer);
		case I4C_NOOP:
			if(cmd->status != I4CS_PARSING)
				return 0;
			cmd->status = I4CS_OK;
			return 0;
		case I4C_SELECT:
			return _context_select(plugin);
		case I4C_STATUS:
			return _context_status(plugin, answer);
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

static int _context_init(AccountPlugin * plugin)
{
	IMAP4 * imap4 = plugin->priv;
	IMAP4Command * cmd = &imap4->queue[0];
	char const * p;
	char const * q;
	gchar * r;

	cmd->status = I4CS_OK;
	if((p = plugin->config[I4CV_USERNAME].value) == NULL || *p == '\0')
		return -1;
	if((q = plugin->config[I4CV_PASSWORD].value) == NULL || *q == '\0')
		return -1;
	r = g_strdup_printf("%s %s %s", "LOGIN", p, q);
	cmd = _imap4_command(plugin, I4C_LOGIN, r);
	g_free(r);
	return (cmd != NULL) ? 0 : -1;
}

static int _context_list(AccountPlugin * plugin, char const * answer)
{
	IMAP4 * imap4 = plugin->priv;
	IMAP4Command * cmd = &imap4->queue[0];
	AccountFolder * folder;
	AccountFolder * parent = cmd->data.list.parent;
	char const * p;
	gchar * q;
	char const haschildren[] = "(\\HasChildren) ";
	int recurse = 0;
	char reference = '\0';
	char buf[64];

	p = answer;
	if(strncmp("OK", p, 2) == 0)
	{
		cmd->status = I4CS_OK;
		return 0;
	}
	if(strncmp("LIST ", p, 5) != 0)
		return -1;
	p += 5;
	if(*p == '(') /* skip flags */
	{
		/* FIXME there may be other flags set */
		if(strncmp(p, haschildren, sizeof(haschildren) - 1) == 0)
			recurse = 1;
		for(p++; *p != '\0' && *p++ != ')';);
	}
	if(*p == ' ') /* skip spaces */
		for(p++; *p != '\0' && *p == ' '; p++);
	if(*p == '\"') /* skip reference */
	{
		if(p[1] != '\0' && p[1] != '"' && p[2] == '"')
			reference = p[1];
		for(p++; *p != '\0' && *p++ != '\"';);
	}
	if(*p == ' ') /* skip spaces */
		for(p++; *p != '\0' && *p == ' '; p++);
	if(*p == '\"' && sscanf(++p, "%63[^\"]", buf) == 1
			&& (folder = _imap4_folder_get_folder(plugin, parent,
					buf)) != NULL)
	{
		buf[63] = '\0';
		/* FIXME escape the mailbox name (double quotes...) */
		q = g_strdup_printf("%s \"%s\" (%s)", "STATUS", buf,
				"MESSAGES RECENT");
		if((cmd = _imap4_command(plugin, I4C_STATUS, q)) != NULL)
			cmd->data.status.folder = folder;
		g_free(q);
		if(cmd != NULL && recurse == 1 && reference != '\0')
		{
			q = g_strdup_printf("%s \"\" \"%s%c%%\"", "LIST", buf,
					reference);
			if((cmd = _imap4_command(plugin, I4C_LIST, q)) != NULL)
				cmd->data.list.parent = folder;
			g_free(q);
		}
	}
	return (cmd != NULL) ? 0 : -1;
}

static int _context_login(AccountPlugin * plugin, char const * answer)
{
	AccountPluginHelper * helper = plugin->helper;
	IMAP4 * imap4 = plugin->priv;
	IMAP4Command * cmd = &imap4->queue[0];
	char const * prefix = plugin->config[I4CV_PREFIX].value;
	gchar * q;

	if(cmd->status != I4CS_PARSING)
		return 0;
	if(strncmp("OK", answer, 2) != 0)
		return -helper->error(helper->account, "Authentication failed",
				1);
	cmd->status = I4CS_OK;
	if((q = g_strdup_printf("%s \"\" \"%s%%\"", "LIST", (prefix != NULL)
					? prefix : "")) == NULL)
		return -1;
	cmd = _imap4_command(plugin, I4C_LIST, q);
	g_free(q);
	if(cmd == NULL)
		return -1;
	cmd->data.list.parent = &imap4->folders;
	return 0;
}

static int _context_select(AccountPlugin * plugin)
{
	IMAP4 * imap4 = plugin->priv;
	IMAP4Command * cmd = &imap4->queue[0];
	AccountFolder * folder;
	AccountMessage * message;
	char buf[32];

	if(cmd->status != I4CS_PARSING)
		return 0;
	cmd->status = I4CS_OK;
	if((folder = cmd->data.select.folder) == NULL)
		return 0; /* XXX really is an error */
	if((message = cmd->data.select.message) == NULL)
		/* FIXME queue commands in batches instead */
		snprintf(buf, sizeof(buf), "%s %s %s", "FETCH", "1:*",
				"BODY.PEEK[HEADER]");
	else
		snprintf(buf, sizeof(buf), "%s %u %s", "FETCH", message->id,
				"BODY.PEEK[]");
	if((cmd = _imap4_command(plugin, I4C_FETCH, buf)) == NULL)
		return -1;
	cmd->data.fetch.folder = folder;
	cmd->data.fetch.message = message;
	cmd->data.fetch.id = (message != NULL) ? message->id : 0;
	cmd->data.fetch.status = I4FS_ID;
	return 0;
}

static int _context_status(AccountPlugin * plugin, char const * answer)
{
	IMAP4 * imap4 = plugin->priv;
	IMAP4Command * cmd = &imap4->queue[0];
	char const * p;
	unsigned int m;
	unsigned int r;

	p = answer;
	if(strncmp("OK", p, 2) == 0)
	{
		cmd->status = I4CS_OK;
		return 0;
	}
	if(strncmp("STATUS ", p, 7) != 0)
		return 0;
	p += 7;
	if(*p == '\"') /* skip reference */
		for(p++; *p != '\0' && *p++ != '\"';);
	if(*p == ' ') /* skip spaces */
		for(p++; *p != '\0' && *p == ' '; p++);
	if(*p != '(')
		return 0;
	if(sscanf(p, "(MESSAGES %u RECENT %u)", &m, &r) != 2)
		return 0;
	/* FIXME implement */
	return 0;
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
		{ "Sent",	FT_SENT		}
	};
	size_t i;
	size_t len;

	if((p = realloc(parent->folders, sizeof(*p) * (parent->folders_cnt
						+ 1))) == NULL)
		return NULL;
	parent->folders = p;
	if((folder = object_new(sizeof(*folder))) == NULL)
		return NULL;
	folder->name = strdup(name);
	if(parent == &imap4->folders)
		for(i = 0; i < sizeof(name_type) / sizeof(*name_type); i++)
			if(strcasecmp(name_type[i].name, name) == 0)
			{
				type = name_type[i].type;
				name = name_type[i].name;
				break;
			}
	/* shorten the name as required if has a parent */
	if(parent->name != NULL && (len = strlen(parent->name)) > 0
			&& strncmp(name, parent->name, len) == 0)
		name = &name[len + 1];
	folder->folder = helper->folder_new(helper->account, folder,
			parent->folder, type, name);
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
static int _connect_channel(AccountPlugin * plugin);

static gboolean _on_connect(gpointer data)
{
	AccountPlugin * plugin = data;
	AccountPluginHelper * helper = plugin->helper;
	IMAP4 * imap4 = plugin->priv;
	char const * hostname;
	char const * p;
	struct hostent * he;
	unsigned short port;
	struct sockaddr_in sa;
	char buf[128];

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	imap4->source = 0;
	/* FIXME report errors */
	if((hostname = plugin->config[I4CV_HOSTNAME].value) == NULL)
		return FALSE;
	if((he = gethostbyname(hostname)) == NULL)
	{
		helper->error(NULL, hstrerror(h_errno), 1);
		return FALSE;
	}
	if((p = plugin->config[I4CV_PORT].value) == NULL)
		return FALSE;
	port = (unsigned long)p;
	if(plugin->config[I4CV_SSL].value != NULL)
		if((imap4->ssl_ctx = SSL_CTX_new(SSLv3_client_method())) == NULL
				|| SSL_CTX_set_cipher_list(imap4->ssl_ctx,
					SSL_DEFAULT_CIPHER_LIST) != 1)
		{
			helper->error(NULL, ERR_error_string(ERR_get_error(),
						buf), 1);
			return FALSE;
		}
	if((imap4->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		helper->error(NULL, strerror(errno), 1);
		return FALSE;
	}
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = *((uint32_t*)he->h_addr_list[0]);
	helper->status(helper->account, "Connecting to %s (%s:%u)", hostname,
			inet_ntoa(sa.sin_addr), port);
	if(fcntl(imap4->fd, F_SETFL, O_NONBLOCK) == -1)
		helper->error(NULL, strerror(errno), 1);
	if((connect(imap4->fd, (struct sockaddr *)&sa, sizeof(sa)) != 0
				&& errno != EINPROGRESS)
			|| _connect_channel(plugin) != 0)
	{
		helper->error(NULL, strerror(errno), 1);
		if(imap4->ssl_ctx != NULL)
			SSL_CTX_free(imap4->ssl_ctx);
		imap4->ssl_ctx = NULL;
		close(imap4->fd);
		imap4->fd = -1;
		return FALSE;
	}
	imap4->wr_source = g_io_add_watch(imap4->channel, G_IO_OUT,
			_on_watch_can_connect, plugin);
	return FALSE;
}

static int _connect_channel(AccountPlugin * plugin)
{
	AccountPluginHelper * helper = plugin->helper;
	IMAP4 * imap4 = plugin->priv;
	GError * error = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	/* prepare queue */
	if((imap4->queue = malloc(sizeof(*imap4->queue))) == NULL)
		return -helper->error(helper->account, strerror(errno), 1);
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
	return 0;
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
	if(imap4->ssl_ctx != NULL)
		SSL_CTX_free(imap4->ssl_ctx);
	imap4->ssl_ctx = NULL;
	if(imap4->fd >= 0)
		close(imap4->fd);
	imap4->fd = -1;
	imap4->source = g_timeout_add(3000, _on_connect, plugin);
	return FALSE;
}


/* on_watch_can_connect */
static gboolean _on_watch_can_connect(GIOChannel * source,
		GIOCondition condition, gpointer data)
{
	AccountPlugin * plugin = data;
	AccountPluginHelper * helper = plugin->helper;
	IMAP4 * imap4 = plugin->priv;
	char buf[128];

	if(condition != G_IO_OUT || source != imap4->channel)
		return FALSE; /* should not happen */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() connected\n", __func__);
#endif
	imap4->wr_source = 0;
	/* setup SSL */
	if(imap4->ssl_ctx != NULL)
	{
		if((imap4->ssl = SSL_new(imap4->ssl_ctx)) == NULL)
		{
			helper->error(NULL, ERR_error_string(ERR_get_error(),
						buf), 1);
			return FALSE;
		}
		if(SSL_set_fd(imap4->ssl, imap4->fd) != 1)
			; /* FIXME handle error */
		SSL_set_connect_state(imap4->ssl);
		/* perform initial handshake */
		imap4->wr_source = g_io_add_watch(imap4->channel, G_IO_OUT,
				_on_watch_can_handshake, plugin);
		return FALSE;
	}
	/* wait for the server's banner */
	imap4->rd_source = g_io_add_watch(imap4->channel, G_IO_IN,
			_on_watch_can_read, plugin);
	return FALSE;
}


/* on_watch_can_handshake */
static gboolean _on_watch_can_handshake(GIOChannel * source,
		GIOCondition condition, gpointer data)
{
	AccountPlugin * plugin = data;
	IMAP4 * imap4 = plugin->priv;
	int res;

	if((condition != G_IO_IN && condition != G_IO_OUT)
			|| source != imap4->channel || imap4->ssl == NULL)
		return FALSE; /* should not happen */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	imap4->wr_source = 0;
	imap4->rd_source = 0;
	if((res = SSL_do_handshake(imap4->ssl)) == 1)
	{
		/* wait for the server's banner */
		imap4->rd_source = g_io_add_watch(imap4->channel, G_IO_IN,
				_on_watch_can_read_ssl, plugin);
		return FALSE;
	}
	else if(res == 0)
	{
		/* FIXME handle error */
		return FALSE;
	}
	res = SSL_get_error(imap4->ssl, res);
	if(res == SSL_ERROR_WANT_WRITE)
		imap4->wr_source = g_io_add_watch(imap4->channel, G_IO_OUT,
				_on_watch_can_handshake, plugin);
	else if(res == SSL_ERROR_WANT_READ)
		imap4->rd_source = g_io_add_watch(imap4->channel, G_IO_IN,
				_on_watch_can_handshake, plugin);
	/* FIXME else handle error */
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
	if(_imap4_parse(plugin) != 0)
	{
		imap4->rd_source = g_idle_add(_on_reset, plugin);
		return FALSE;
	}
	if(imap4->queue_cnt == 0)
	{
		imap4->rd_source = 0;
		return FALSE;
	}
	cmd = &imap4->queue[0];
	if(cmd->buf_cnt == 0)
	{
		if(cmd->status == I4CS_SENT || cmd->status == I4CS_PARSING)
			return TRUE;
		else if(cmd->status == I4CS_OK || cmd->status == I4CS_ERROR)
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


/* on_watch_can_read_ssl */
static gboolean _on_watch_can_read_ssl(GIOChannel * source,
		GIOCondition condition, gpointer data)
{
	AccountPlugin * plugin = data;
	IMAP4 * imap4 = plugin->priv;
	char * p;
	int cnt;
	IMAP4Command * cmd;
	char buf[128];

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(condition != G_IO_IN || source != imap4->channel)
		return FALSE; /* should not happen */
	if((p = realloc(imap4->rd_buf, imap4->rd_buf_cnt + 256)) == NULL)
		return TRUE; /* XXX retries immediately (delay?) */
	imap4->rd_buf = p;
	if((cnt = SSL_read(imap4->ssl, &imap4->rd_buf[imap4->rd_buf_cnt], 256))
			<= 0)
	{
		if(cnt < 0)
		{
			ERR_error_string(SSL_get_error(imap4->ssl, cnt), buf);
			plugin->helper->error(NULL, buf, 1);
		}
		imap4->rd_source = g_idle_add(_on_reset, plugin);
		return FALSE;
	}
#ifdef DEBUG
	fprintf(stderr, "%s", "DEBUG: IMAP4 SERVER: ");
	fwrite(&imap4->rd_buf[imap4->rd_buf_cnt], sizeof(*p), cnt, stderr);
#endif
	imap4->rd_buf_cnt += cnt;
	if(_imap4_parse(plugin) != 0)
	{
		imap4->rd_source = g_idle_add(_on_reset, plugin);
		return FALSE;
	}
	if(imap4->queue_cnt == 0)
	{
		imap4->rd_source = 0;
		return FALSE;
	}
	cmd = &imap4->queue[0];
	if(cmd->buf_cnt == 0)
	{
		if(cmd->status == I4CS_SENT || cmd->status == I4CS_PARSING)
			return TRUE;
		else if(cmd->status == I4CS_OK || cmd->status == I4CS_ERROR)
			memmove(cmd, &imap4->queue[1], sizeof(*cmd)
					* --imap4->queue_cnt);
	}
	imap4->rd_source = 0;
	if(imap4->queue_cnt == 0)
		imap4->source = g_timeout_add(30000, _on_noop, plugin);
	else
		imap4->wr_source = g_io_add_watch(imap4->channel, G_IO_OUT,
				_on_watch_can_write_ssl, plugin);
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

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
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


/* on_watch_can_write_ssl */
static gboolean _on_watch_can_write_ssl(GIOChannel * source,
		GIOCondition condition, gpointer data)
{
	AccountPlugin * plugin = data;
	IMAP4 * imap4 = plugin->priv;
	IMAP4Command * cmd = &imap4->queue[0];
	int cnt;
	char * p;
	char buf[128];

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(condition != G_IO_OUT || source != imap4->channel
			|| imap4->queue_cnt == 0 || cmd->buf_cnt == 0)
		return FALSE; /* should not happen */
	if((cnt = SSL_write(imap4->ssl, cmd->buf, cmd->buf_cnt)) <= 0)
	{
		if(cnt < 0)
		{
			ERR_error_string(SSL_get_error(imap4->ssl, cnt), buf);
			plugin->helper->error(NULL, buf, 1);
		}
		imap4->wr_source = g_idle_add(_on_reset, plugin);
		return FALSE;
	}
#ifdef DEBUG
	fprintf(stderr, "%s", "DEBUG: IMAP4 CLIENT: ");
	fwrite(cmd->buf, sizeof(*p), cnt, stderr);
#endif
	cmd->buf_cnt -= cnt;
	memmove(cmd->buf, &cmd->buf[cnt], cmd->buf_cnt);
	if((p = realloc(cmd->buf, cmd->buf_cnt)) != NULL)
		cmd->buf = p; /* we can ignore errors... */
	else if(cmd->buf_cnt == 0)
		cmd->buf = NULL; /* ...except when it's not one */
	if(cmd->buf_cnt > 0)
		return TRUE;
	cmd->status = I4CS_SENT;
	imap4->wr_source = 0;
	if(imap4->rd_source == 0)
		imap4->rd_source = g_io_add_watch(imap4->channel, G_IO_IN,
				_on_watch_can_read_ssl, plugin);
	return FALSE;
}
