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



#ifndef MAILER_SRC_MAILER_H
# define MAILER_SRC_MAILER_H

# include <System.h>
# include <gtk/gtk.h>
# include "Mailer.h"
# include "account.h"


/* Mailer */
/* types */
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


/* constants */
# define MAILER_CONFIG_FILE	".mailer"

# define MAILER_MESSAGES_FONT	"Monospace 9"


/* functions */
Mailer * mailer_new(void);
void mailer_delete(Mailer * mailer);

/* accessors */
char const * mailer_get_config(Mailer * mailer, char const * variable);
void mailer_set_status(Mailer * mailer, char const * status);

/* useful */
int mailer_error(Mailer * mailer, char const * message, int ret);

/* accounts */
int mailer_account_add(Mailer * mailer, Account * account);
#if 0 /* FIXME deprecate? */
int mailer_account_disable(Mailer * mailer, Account * account);
int mailer_account_enable(Mailer * mailer, Account * account);
#endif
/* FIXME implement
int mailer_account_remove(Mailer * mailer, Account * account); */

/* plug-ins */
int mailer_load(Mailer * mailer, char const * plugin);
int mailer_unload(Mailer * mailer, char const * plugin);

void mailer_compose(Mailer * mailer);

/* selection */
void mailer_delete_selected(Mailer * mailer);

void mailer_open_selected_source(Mailer * mailer);

void mailer_reply_selected(Mailer * mailer);
void mailer_reply_selected_to_all(Mailer * mailer);

void mailer_select_all(Mailer * mailer);
void mailer_unselect_all(Mailer * mailer);

/* clipboard */
void mailer_cut(Mailer * mailer);
void mailer_copy(Mailer * mailer);
void mailer_paste(Mailer * mailer);

/* interface */
void mailer_show_about(Mailer * mailer, gboolean show);
void mailer_show_body(Mailer * mailer, gboolean show);
void mailer_show_headers(Mailer * mailer, gboolean show);
void mailer_show_plugins(Mailer * mailer, gboolean show);
void mailer_show_preferences(Mailer * mailer, gboolean show);

#endif /* !MAILER_SRC_MAILER_H */
