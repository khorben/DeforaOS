/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
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



#ifndef DESKTOP_MAILER_ACCOUNT_H
# define DESKTOP_MAILER_ACCOUNT_H

# include <openssl/ssl.h>
# include "folder.h"
# include "message.h"


/* Account */
/* types */
typedef struct _Account Account;

/* AccountIdentity */
typedef struct _AccountIdentity
{
	char * from;
	char * email;
	char * signature;
} AccountIdentity;


/* AccountConfig */
typedef enum _AccountConfigType
{
	ACT_NONE = 0,
	/* values */
	ACT_STRING,
	ACT_PASSWORD,
	ACT_FILE,
	ACT_UINT16,
	ACT_BOOLEAN,
	/* layout */
	ACT_SEPARATOR
} AccountConfigType;

typedef struct _AccountConfig
{
	char const * name;
	char const * title;
	AccountConfigType type;
	void * value;
} AccountConfig;


/* AccountPlugin */
typedef struct _AccountPluginHelper
{
	Account * account;
	/* accessors */
	SSL_CTX * (*get_ssl_context)(Account * account);
	/* useful */
	int (*error)(Account * account, char const * message, int ret);
	void (*status)(Account * account, char const * format, ...);
	/* authentication */
	char * (*authenticate)(Account * account, char const * message);
	int (*confirm)(Account * account, char const * message);
	/* folders */
	Folder * (*folder_new)(Account * account, AccountFolder * folder,
			Folder * parent, FolderType type, char const * name);
	void (*folder_delete)(Folder * folder);
	/* messages */
	Message * (*message_new)(Account * account, Folder * folder,
			AccountMessage * message);
	void (*message_delete)(Message * message);
	int (*message_set_header)(Message * message, char const * header);
	int (*message_set_body)(Message * message, char const * buf, size_t cnt,
			int append);
} AccountPluginHelper;

typedef struct _AccountPlugin AccountPlugin;

typedef const struct _AccountPluginDefinition
{
	char const * type;
	char const * name;
	char const * icon;
	char const * description;
	AccountConfig const * config;
	AccountPlugin * (*init)(AccountPluginHelper * helper);
	int (*destroy)(AccountPlugin * plugin);
	AccountConfig * (*get_config)(AccountPlugin * plugin);
	char * (*get_source)(AccountPlugin * plugin, AccountFolder * folder,
			AccountMessage * message);
	int (*refresh)(AccountPlugin * plugin, AccountFolder * folder,
			AccountMessage * message);
} AccountPluginDefinition;

#endif /* !DESKTOP_MAILER_ACCOUNT_H */
