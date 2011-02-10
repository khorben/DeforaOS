/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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



#include <stdio.h>
#include <libintl.h>


/* macros */
#ifndef _
# define _(string) gettext(string)
#endif
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
			context->suggested_action == GDK_ACTION_COPY
			? _("copying") : _("moving"), _(" to \""), dest,
			"\":\n");
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
	GList * a;
	char const ** p;
	GError * error = NULL;

	if(args == NULL)
		return 0;
	for(a = args; a != NULL; a = a->next)
	{
		if(a->data == NULL)
			continue;
		if((p = realloc(argv, sizeof(*argv) * (i + 2))) == NULL)
			break;
		argv = p;
		argv[i++] = a->data;
	}
	if(a != NULL)
	{
		free(argv);
		return -error_set_code(1, "%s: %s", program, strerror(errno));
	}
	if(argv == NULL)
		return 0;
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
	if(g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL,
				NULL, &error) != TRUE)
		return -error_set_code(1, "%s", error->message);
	return 0;
}
#endif /* COMMON_EXEC */


#ifdef COMMON_SYMLINK
/* common_symlink */
static int _common_symlink(GtkWidget * window, char const * cur)
{
	static char const * newsymlink = NULL;
	int ret = 0;
	size_t len;
	char * path;
	GtkWidget * dialog;
	GtkWidget * hbox;
	GtkWidget * widget;
	char const * to = NULL;

	if(newsymlink == NULL)
		newsymlink = _("New symbolic link");
	len = strlen(cur) + strlen(newsymlink) + 2;
	if((path = malloc(len)) == NULL)
		return 1;
	snprintf(path, len, "%s/%s", cur, newsymlink);
	dialog = gtk_dialog_new_with_buttons(newsymlink, GTK_WINDOW(window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(_("Destination: "));
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	widget = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	gtk_widget_show_all(hbox);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, TRUE, TRUE,
			4);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
		to = gtk_entry_get_text(GTK_ENTRY(widget));
	if(to != NULL && strlen(to) > 0 && symlink(to, path) != 0)
		ret = 1;
	gtk_widget_destroy(dialog);
	free(path);
	return ret;
}
#endif /* COMMON_SYMLINK */
