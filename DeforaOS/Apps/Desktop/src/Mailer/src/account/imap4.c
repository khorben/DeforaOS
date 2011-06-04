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



#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <string.h>
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
	I4C_INIT = 0
} IMAP4Context;

typedef struct _IMAP4Command
{
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
static int _imap4_init(AccountPlugin * plugin);
static int _imap4_destroy(AccountPlugin * plugin);

/* callbacks */
static gboolean _on_idle(gpointer data);


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
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	/* FIXME implement */
	return FALSE;
}

static gboolean _idle_channel(AccountPlugin * plugin)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	/* FIXME implement */
	return FALSE;
}
