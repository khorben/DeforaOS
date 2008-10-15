/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
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


/* types */
#ifdef COMMON_MENU
struct _menu
{
	const char * name;
	GtkSignalFunc callback;
	const char * stock;
	unsigned int accel;
};

struct _menubar
{
	const char * name;
	struct _menu * menu;
};
#endif


/* prototypes */
#ifdef COMMON_DND
static int _common_drag_data_received(GdkDragContext * context,
		GtkSelectionData * seldata, char * dest);
#endif

#ifdef COMMON_EXEC
static int _common_exec(char * program, char * flags, GList * args);
#endif

#ifdef COMMON_MENU
static GtkWidget * _common_new_menubar(GtkWindow * window, struct _menubar * mb,
		gpointer data);
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
#endif /* COMMON_EXEC */


#ifdef COMMON_MENU
static GtkWidget * _common_new_menubar(GtkWindow * window, struct _menubar * mb,
		gpointer data)
{
	GtkWidget * tb_menubar;
	GtkAccelGroup * group;
	GtkWidget * menu;
	GtkWidget * menubar;
	GtkWidget * menuitem;
	GtkWidget * image;
	unsigned int i;
	unsigned int j;
	struct _menu * p;

	tb_menubar = gtk_menu_bar_new();
	group = gtk_accel_group_new();
	for(i = 0; mb[i].name != NULL; i++)
	{
		menubar = gtk_menu_item_new_with_mnemonic(mb[i].name);
		menu = gtk_menu_new();
		for(j = 0; mb[i].menu[j].name != NULL; j++)
		{
			p = &mb[i].menu[j];
			if(p->name[0] == '\0')
				menuitem = gtk_separator_menu_item_new();
			else if(p->stock == NULL)
				menuitem = gtk_menu_item_new_with_mnemonic(
						p->name);
			else if(strncmp(p->stock, "gtk-", 4) == 0)
				menuitem = gtk_image_menu_item_new_from_stock(
						p->stock, NULL);
			else
			{
				image = gtk_image_new_from_icon_name(p->stock,
						GTK_ICON_SIZE_MENU);
				menuitem
					= gtk_image_menu_item_new_with_mnemonic(
							p->name);
				gtk_image_menu_item_set_image(
						GTK_IMAGE_MENU_ITEM(menuitem),
						image);
			}
			if(p->callback != NULL)
				g_signal_connect(G_OBJECT(menuitem), "activate",
						G_CALLBACK(p->callback), data);
			else
				gtk_widget_set_sensitive(menuitem, FALSE);
			if(p->accel != 0)
				gtk_widget_add_accelerator(menuitem, "activate",
						group, p->accel,
						GDK_CONTROL_MASK,
						GTK_ACCEL_VISIBLE);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		}
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(menubar), menu);
		gtk_menu_bar_append(GTK_MENU_BAR(tb_menubar), menubar);
	}
	gtk_window_add_accel_group(GTK_WINDOW(window), group);
	return tb_menubar;
}
#endif /* COMMON_MENU */
