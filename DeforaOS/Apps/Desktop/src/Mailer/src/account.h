/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#ifndef MAILER_ACCOUNT_H
# define MAILER_ACCOUNT_H


/* types */
typedef struct _Account
{
	char * name;
} Account;

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


/* functions */
Account * account_new(char const * type, char const * name);
void account_delete(Account * account);

/* useful */
AccountFolder ** account_folders(Account * account);

#endif /* !MAILER_ACCOUNT_H */
