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



#ifndef DESKTOP_MAILER_FOLDER_H
# define DESKTOP_MAILER_FOLDER_H


/* Folder */
/* types */
typedef struct _Folder Folder;

typedef struct _AccountFolder AccountFolder;

typedef enum _FolderType
{
	FT_INBOX = 0,
	FT_DRAFTS,
	FT_SENT,
	FT_TRASH,
	FT_FOLDER
} FolderType;
# define FT_LAST FT_FOLDER
# define FT_COUNT (FT_LAST + 1)

#endif /* !DESKTOP_MAILER_FOLDER_H */
