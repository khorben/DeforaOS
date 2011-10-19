/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the authors nor the names of the contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY ITS AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE. */



#include <unistd.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <Devel/Asm.h>


/* functions */
/* gdeasm */
static void _gdeasm_open(char const * filename, GtkListStore * store);
/* callbacks */
static gboolean _gdeasm_on_closex(void);
static void _gdeasm_on_open(gpointer data);
static void _gdeasm_callback(void * priv, ArchInstructionCall * call);
static void _callback_dregister(char * buf, size_t size, ArchOperand * ao);
static void _callback_dregister2(char * buf, size_t size, ArchOperand * ao);
static void _callback_immediate(char * buf, size_t size, ArchOperand * ao);

static int _gdeasm(char const * filename)
{
	GtkListStore * store;
	GtkWidget * window;
	GtkWidget * vbox;
	GtkWidget * toolbar;
	GtkToolItem * toolitem;
	GtkWidget * scrolled;
	GtkWidget * treeview;
	GtkTreeViewColumn * column;
	char const * headers[] = { "Offset", "Instruction", "Operand",
		"Operand", "Operand" };
	size_t i;

	store = gtk_list_store_new(5, G_TYPE_STRING, G_TYPE_STRING,
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);
	g_signal_connect(window, "delete-event", G_CALLBACK(_gdeasm_on_closex),
			NULL);
	vbox = gtk_vbox_new(FALSE, 0);
	toolbar = gtk_toolbar_new();
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
	g_signal_connect_swapped(toolitem, "clicked", G_CALLBACK(
				_gdeasm_on_open), store);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, TRUE, 0);
	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	for(i = 0; i < sizeof(headers) / sizeof(*headers); i++)
	{
		column = gtk_tree_view_column_new_with_attributes(headers[i],
				gtk_cell_renderer_text_new(), "text", i, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	}
	gtk_container_add(GTK_CONTAINER(scrolled), treeview);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
	if(filename != NULL)
		_gdeasm_open(filename, store);
	gtk_main();
	return 0;
}

static void _gdeasm_callback(void * priv, ArchInstructionCall * call)
{
	GtkListStore * store = priv;
	GtkTreeIter iter;
	char buf[32];
	size_t i;
	ArchOperand * ao;
	char const * name;

	gtk_list_store_append(store, &iter);
	snprintf(buf, sizeof(buf), "%08lx", call->base);
	gtk_list_store_set(store, &iter, 0, buf, 1, call->name, -1);
	for(i = 0; i < call->operands_cnt; i++)
	{
		ao = &call->operands[i];
		switch(AO_GET_TYPE(ao->definition))
		{
			case AOT_DREGISTER:
				_callback_dregister(buf, sizeof(buf), ao);
				break;
			case AOT_DREGISTER2:
				_callback_dregister2(buf, sizeof(buf), ao);
				break;
			case AOT_IMMEDIATE:
				_callback_immediate(buf, sizeof(buf), ao);
				break;
			case AOT_REGISTER:
				name = call->operands[i].value._register.name;
				snprintf(buf, sizeof(buf), "%%%s", name);
				break;
			default:
				buf[0] = '\0';
				break;
		}
		gtk_list_store_set(store, &iter, i + 2, buf, -1);
	}
}

static void _callback_dregister(char * buf, size_t size, ArchOperand * ao)
{
	char const * name;

	name = ao->value.dregister.name;
	if(ao->value.dregister.offset == 0)
		snprintf(buf, size, "[%%%s]", name);
	else
		snprintf(buf, size, "[%%%s + $0x%lx]", name,
				(unsigned long)ao->value.dregister.offset);
}

static void _callback_dregister2(char * buf, size_t size, ArchOperand * ao)
{
	snprintf(buf, size, "[%%%s + %%%s]", ao->value.dregister2.name,
			ao->value.dregister2.name2);
}

static void _callback_immediate(char * buf, size_t size, ArchOperand * ao)
{
	snprintf(buf, size, "%s$0x%lx", ao->value.immediate.negative
			? "-" : "", (unsigned long)ao->value.immediate.value);
}

static void _gdeasm_open(char const * filename, GtkListStore * store)
{
	Asm * a;

	if((a = asm_new(NULL, NULL)) == NULL)
		return;
#if 0 /* FIXME requires patches to libasm */
	asm_open_deassemble(a, filename, 0, _gdeasm_callback, store);
#else
	asm_open_deassemble(a, filename, 0);
#endif
	asm_close(a);
}

static gboolean _gdeasm_on_closex(void)
{
	gtk_main_quit();
	return TRUE;
}

static void _gdeasm_on_open(gpointer data)
{
	GtkListStore * store = data;
	GtkWidget * widget;
	char * filename = NULL;

	widget = gtk_file_chooser_dialog_new("Open file...", NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, NULL);
	if(gtk_dialog_run(GTK_DIALOG(widget)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					widget));
	gtk_widget_destroy(widget);
	if(filename == NULL)
		return;
	gtk_list_store_clear(store);
	_gdeasm_open(filename, store);
	g_free(filename);
}


/* usage */
static int _usage(void)
{
	fputs("Usage: gdeasm filename\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind != argc && optind + 1 != argc)
		return _usage();
	return (_gdeasm(argv[optind]) == 0) ? 0 : 2;
}
