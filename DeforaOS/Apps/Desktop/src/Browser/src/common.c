/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
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


/* macros */
#define min(a, b) ((a) < (b)) ? (a) : (b)


/* prototypes */
#ifdef COMMON_DND
static int _common_drag_data_received(GdkDragContext * context,
		GtkSelectionData * seldata, char * dest);
#endif

#ifdef COMMON_EXEC
static int _common_exec(char const * program, char const * flags, GList * args);
#endif


/* functions */
#ifdef COMMON_DND
/* common_drag_data_received */
static int _common_drag_data_received(GdkDragContext * context,
		GtkSelectionData * seldata, char * dest)
{
	int ret = 0;
	size_t len;
	size_t i;
	GList * selection = NULL;
#ifdef DEBUG
	GList * s;
#endif

	if(seldata->length <= 0 || seldata->data == NULL)
		return 0;
	len = seldata->length;
	for(i = 0; i < len; i += strlen((char*)&seldata->data[i]) + 1)
		selection = g_list_append(selection, &seldata->data[i]);
#ifdef DEBUG
	fprintf(stderr, "%s%s%s%s%s", "DEBUG: ",
			context->suggested_action == GDK_ACTION_COPY ? "copying"
			: "moving", " to \"", dest, "\":\n");
	for(s = selection; s != NULL; s = s->next)
		fprintf(stderr, "DEBUG: \"%s\"\n", (char*)s->data);
#else
	selection = g_list_append(selection, dest);
	if(context->suggested_action == GDK_ACTION_COPY)
		ret = _common_exec("copy", "-iR", selection);
	else if(context->suggested_action == GDK_ACTION_MOVE)
		ret = _common_exec("move", "-i", selection);
#endif
	g_list_free(selection);
	return ret;
}
#endif /* COMMON_DND */


#ifdef COMMON_EXEC
/* common_exec */
static int _common_exec(char const * program, char const * flags, GList * args)
{
	unsigned long i = flags != NULL ? 3 : 2;
	char const ** argv = NULL;
	pid_t pid;
	GList * a;
	char const ** p;

	if(args == NULL)
		return 0;
	if((pid = fork()) == -1)
		return 1;
	else if(pid != 0) /* the parent returns */
		return 0;
	for(a = args; a != NULL; a = a->next)
	{
		if(a->data == NULL)
			continue;
		if((p = realloc(argv, sizeof(*argv) * (i + 2))) == NULL)
		{
			fprintf(stderr, "%s%s%s%s%s", PACKAGE ": ", program,
					": ", strerror(errno), "\n");
			exit(2);
		}
		argv = p;
		argv[i++] = a->data;
	}
	if(argv == NULL)
		exit(0);
#ifdef DEBUG
	argv[0] = "echo";
#else
	argv[0] = program;
#endif
	argv[i] = NULL;
	i = 1;
	if(flags != NULL)
		argv[i++] = flags;
	argv[i] = "--";
	execvp(argv[0], argv);
	fprintf(stderr, "%s%s%s%s\n", PACKAGE ": ", argv[0], ": ", strerror(
				errno));
	exit(2);
}
#endif /* COMMON_EXEC */
