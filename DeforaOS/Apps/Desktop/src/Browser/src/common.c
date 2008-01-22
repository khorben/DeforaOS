/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
/* Browser is not free software; you can redistribute it and/or modify it
 * under the terms of the Creative Commons Attribution-NonCommercial-ShareAlike
 * 3.0 Unported as published by the Creative Commons organization.
 *
 * Browser is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with Browser; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */


/* prototypes */
static int _common_drag_data_received(GdkDragContext * context,
		GtkSelectionData * seldata, char * dest);
static int _common_exec(char * program, char * flags, GList * args);


/* functions */
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
		ret = _common_exec("copy", "-ir", selection);
	else if(context->suggested_action == GDK_ACTION_MOVE)
		ret = _common_exec("move", "-i", selection);
#endif
	g_list_free(selection);
	return ret;
}


/* common_exec */
static int _common_exec(char * program, char * flags, GList * args)
{
	unsigned long i = flags != NULL ? 3 : 2;
	char ** argv = NULL;
	pid_t pid;
	GList * a;
	char ** p;

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
