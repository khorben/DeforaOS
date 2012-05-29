/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop libDesktop */
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



#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <glib.h>
#include <System.h>
#include "Desktop.h"


/* Help */
/* desktop_help_contents */
int desktop_help_contents(char const * package)
{
	char * argv[] = { "helper", "helper", "--", NULL, NULL };
	GSpawnFlags flags = G_SPAWN_SEARCH_PATH | G_SPAWN_FILE_AND_ARGV_ZERO;
	GError * error = NULL;

	if((argv[3] = strdup(package)) == NULL)
		return -error_set_code(1, "%s", strerror(errno));
	if(g_spawn_async(NULL, argv, NULL, flags, NULL, NULL, NULL, &error)
			!= TRUE)
	{
		free(argv[2]);
		error_set_code(1, "%s", error->message);
		g_error_free(error);
		return -1;
	}
	free(argv[3]);
	return 0;
}
