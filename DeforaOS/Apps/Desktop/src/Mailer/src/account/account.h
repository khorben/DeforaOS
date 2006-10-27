/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#ifndef MAILER_ACCOUNT_H
# define MAILER_ACCOUNT_H


/* types */
typedef enum _AccountConfigType
{
	ACT_NONE = 0,
	ACT_STRING
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
	char const * name;
	AccountConfig * config;
	AccountFolder ** (*folders)(void);
} AccountPlugin;
typedef struct _Account
{
	char * name;
	char * title;
	void * handle;
	AccountPlugin * plugin;
} Account;


/* functions */
Account * account_new(char const * type, char const * name);
void account_delete(Account * account);

/* useful */
/* FIXME wrong we just need receive, then it calls callbacks */
AccountFolder ** account_folders(Account * account);

#endif /* !MAILER_ACCOUNT_H */
