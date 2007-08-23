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



#ifndef MAILER_ACCOUNT_H
# define MAILER_ACCOUNT_H


/* types */
typedef struct _AccountIdentity
{
	char * from;
	char * email;
} AccountIdentity;

typedef enum _AccountConfigType
{
	ACT_NONE = 0,
	ACT_STRING,
	ACT_PASSWORD,
	ACT_FILE,
	ACT_UINT16,
	ACT_BOOLEAN
} AccountConfigType;
typedef struct _AccountConfig
{
	char * name;
	char * title;
	AccountConfigType type;
	void * value;
} AccountConfig;

typedef enum _AccountFolderType
{
	AF_INBOX = 0,
	AF_DRAFTS,
	AF_SENT,
	AF_TRASH,
	AF_FOLDER
} AccountFolderType;
typedef struct _AccountFolder
{
	AccountFolderType type;
	char * name;
} AccountFolder;

typedef struct _AccountPlugin
{
	char const * type;
	char const * name;
	AccountConfig * config;
	AccountFolder ** (*folders)(void);
} AccountPlugin;

typedef struct _Account
{
	char * name;
	char * title;
	int enabled;
	AccountIdentity * identity;
	void * handle;
	AccountPlugin * plugin;
} Account;


/* functions */
Account * account_new(char const * type, char const * name);
void account_delete(Account * account);

/* accessors */
int account_set_title(Account * account, char const * title);

/* useful */
int account_disable(Account * account);
int account_enable(Account * account);
/* FIXME wrong we just need receive, then it calls callbacks */
AccountFolder ** account_folders(Account * account);

#endif /* !MAILER_ACCOUNT_H */
