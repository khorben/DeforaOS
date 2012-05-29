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
int desktop_help_contents(char const * package, char const * command)
{
	char * argv[] = { "helper", "helper", "-p", NULL, "--", NULL, NULL };
	GSpawnFlags flags = G_SPAWN_SEARCH_PATH | G_SPAWN_FILE_AND_ARGV_ZERO;
	GError * error = NULL;

	if(package == NULL)
		return -1;
	if(command == NULL)
		command = "index";
	argv[3] = strdup(package);
	argv[5] = strdup(command);
	if(argv[3] == NULL || argv[5] == NULL)
	{
		free(argv[3]);
		free(argv[5]);
		return -error_set_code(1, "%s", strerror(errno));
	}
	g_spawn_async(NULL, argv, NULL, flags, NULL, NULL, NULL, &error);
	free(argv[3]);
	free(argv[5]);
	if(error != NULL)
	{
		error_set_code(1, "%s", error->message);
		g_error_free(error);
		return -1;
	}
	return 0;
}
