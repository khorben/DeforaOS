/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
# include "account/account.h"
# include "../config.h"


/* Mailer */
/* defaults */
# ifndef PLUGINDIR
#  define PLUGINDIR		LIBDIR "/Mailer"
# endif

# define MAILER_CONFIG_FILE	".mailer"

# define MAILER_MESSAGES_FONT	"Monospace 8"


/* types */
enum
{
	MF_COL_ACCOUNT = 0, MF_COL_ENABLED, MF_COL_DELETE, MF_COL_FOLDER,
	MF_COL_ICON, MF_COL_NAME
};
# define MF_COL_LAST MF_COL_NAME
# define MF_COL_COUNT (MF_COL_LAST + 1)

enum
{
	MH_COL_ACCOUNT = 0, MH_COL_FOLDER, MH_COL_MESSAGE, MH_COL_PIXBUF,
	MH_COL_SUBJECT, MH_COL_FROM, MH_COL_TO, MH_COL_DATE,
	MH_COL_DATE_DISPLAY, MH_COL_READ
};
# define MH_COL_LAST MH_COL_READ
# define MH_COL_COUNT (MH_COL_LAST + 1)


/* functions */
Mailer * mailer_new(void);
void mailer_delete(Mailer * mailer);

/* accessors */
char const * mailer_get_config(Mailer * mailer, char const * variable);

/* useful */
int mailer_error(Mailer * mailer, char const * message, int ret);

int mailer_account_add(Mailer * mailer, Account * account);
#if 0 /* FIXME deprecate? */
int mailer_account_disable(Mailer * mailer, Account * account);
int mailer_account_enable(Mailer * mailer, Account * account);
#endif
/* FIXME implement
int mailer_account_remove(Mailer * mailer, Account * account); */

void mailer_show_about(Mailer * mailer, gboolean show);
void mailer_show_preferences(Mailer * mailer, gboolean show);

#endif /* !MAILER_SRC_MAILER_H */
