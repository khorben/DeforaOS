/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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



#ifndef MAILER_MAILER_H
# define MAILER_MAILER_H

# include <gtk/gtk.h>


/* Mailer */
typedef struct _Mailer Mailer;


/* AccountIdentity */
typedef struct _AccountIdentity
{
	char * from;
	char * email;
} AccountIdentity;


/* AccountConfig */
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


/* AccountMessage */
typedef struct _AccountMessage AccountMessage;


/* AccountFolderType */
typedef enum _AccountFolderType
{
	AFT_INBOX = 0,
	AFT_DRAFTS,
	AFT_SENT,
	AFT_TRASH,
	AFT_FOLDER
} AccountFolderType;
# define AFT_LAST AFT_FOLDER
# define AFT_COUNT (AFT_LAST + 1)

typedef struct _AccountFolder
{
	AccountFolderType type;
	char * name;
	GtkListStore * store;
	void * data;
} AccountFolder;


/* AccountPlugin */
typedef struct _AccountPluginHelper
{
	Mailer * mailer;
	GtkIconTheme * theme;
	GdkPixbuf * mail_read;
	GdkPixbuf * mail_unread;
} AccountPluginHelper;

typedef struct _AccountPlugin
{
	AccountPluginHelper * helper;
	char const * type;
	char const * name;
	AccountConfig * config;
	int (*init)(GtkTreeStore * store, GtkTreeIter * parent,
			GtkTextBuffer * buffer);
	int (*quit)(void);
	int (*select)(AccountFolder * folder, AccountMessage * message);
	void * priv;
} AccountPlugin;

#endif /* !MAILER_MAILER_H */
