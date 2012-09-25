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



#ifndef DESKTOP_MAILER_FOLDER_H
# define DESKTOP_MAILER_FOLDER_H

# include "mailer.h"


/* Folder */
/* types */
typedef struct _MailerFolder Folder;

typedef struct _AccountFolder AccountFolder;

typedef enum _MailerFolderType
{
	FT_INBOX = 0,
	FT_DRAFTS,
	FT_SENT,
	FT_TRASH,
	FT_FOLDER
} FolderType;
# define FT_LAST FT_FOLDER
# define FT_COUNT (FT_LAST + 1)


/* public */
/* functions */
/* accessors */
char const * folder_get_name(MailerFolder * folder);
FolderType folder_get_type(MailerFolder * folder);
void folder_set_type(MailerFolder * folder, FolderType type);

#endif /* !DESKTOP_MAILER_FOLDER_H */
