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



#ifndef DESKTOP_MAILER_MAILER_H
# define DESKTOP_MAILER_MAILER_H


/* Mailer */
/* types */
typedef struct _Mailer Mailer;

typedef enum _MailerFolderColumn
{
	MFC_ACCOUNT = 0, MFC_ENABLED, MFC_DELETE, MFC_FOLDER, MFC_ICON, MFC_NAME
} MailerFolderColumn;
# define MFC_LAST MFC_NAME
# define MFC_COUNT (MFC_LAST + 1)

typedef enum _MailerHeaderColumn
{
	MHC_ACCOUNT = 0, MHC_FOLDER, MHC_MESSAGE, MHC_ICON, MHC_SUBJECT,
	MHC_FROM, MHC_FROM_EMAIL, MHC_TO, MHC_TO_EMAIL, MHC_DATE,
	MHC_DATE_DISPLAY, MHC_READ, MHC_WEIGHT
} MailerHeaderColumn;
# define MHC_LAST MHC_WEIGHT
# define MHC_COUNT (MHC_LAST + 1)

/* folders */
typedef struct _MailerFolder MailerFolder;

/* messages */
typedef struct _MailerMessage MailerMessage;

#endif /* !DESKTOP_MAILER_MAILER_H */
