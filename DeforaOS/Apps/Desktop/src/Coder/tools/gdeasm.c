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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <Devel/Asm.h>


/* gdeasm */
/* private */
/* types */
typedef struct _GDeasm
{
	char * arch;
	char * format;

	/* widgets */
	GtkTreeStore * store;
} GDeasm;


/* prototypes */
static GDeasm * _gdeasm_new(char const * arch, char const * format,
		char const * filename);
static void _gdeasm_delete(GDeasm * gdeasm);

static int _gdeasm_open(GDeasm * gdeasm, char const * filename, int raw);


/* callbacks */
static gboolean _gdeasm_on_closex(void);
static void _gdeasm_on_open(gpointer data);

static int _usage(void);


/* functions */
/* gdeasm_new */
/* callbacks */
/* parsing */
static GDeasm * _gdeasm_new(char const * arch, char const * format,
		char const * filename)
{
	GDeasm * gdeasm;
	GtkWidget * window;
	GtkWidget * vbox;
	GtkWidget * toolbar;
	GtkToolItem * toolitem;
	GtkWidget * scrolled;
	GtkWidget * treeview;
	GtkTreeViewColumn * column;
	char const * headers[] = { "Offset", "Instruction", "Operand",
		"Operand", "Operand", "Operand", "Operand" };
	size_t i;

	if((gdeasm = malloc(sizeof(*gdeasm))) == NULL)
		return NULL;
	gdeasm->arch = (arch != NULL) ? strdup(arch) : NULL;
	gdeasm->format = (format != NULL) ? strdup(format) : NULL;
	gdeasm->store = gtk_tree_store_new(7, G_TYPE_STRING, G_TYPE_STRING,
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
			G_TYPE_STRING, G_TYPE_STRING);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);
	g_signal_connect_swapped(window, "delete-event", G_CALLBACK(
				_gdeasm_on_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 0);
	toolbar = gtk_toolbar_new();
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
	g_signal_connect_swapped(toolitem, "clicked", G_CALLBACK(
				_gdeasm_on_open), gdeasm);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, TRUE, 0);
	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(gdeasm->store));
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
		_gdeasm_open(gdeasm, filename, 0);
	return gdeasm;
}


/* gdeasm_delete */
static void _gdeasm_delete(GDeasm * gdeasm)
{
	free(gdeasm->arch);
	free(gdeasm->format);
	free(gdeasm);
}


/* gdeasm_open */
static int _open_code(GDeasm * gdeasm, AsmCode * af);
static int _open_code_section(GDeasm * gdeasm, AsmCode * code,
		AsmSection * section);
static void _open_instruction(GDeasm * gdeasm, GtkTreeIter * parent,
		ArchInstructionCall * call);
static void _open_parse_dregister(char * buf, size_t size, ArchOperand * ao);
static void _open_parse_dregister2(char * buf, size_t size, ArchOperand * ao);
static void _open_parse_immediate(char * buf, size_t size, ArchOperand * ao);

static int _gdeasm_open(GDeasm * gdeasm, char const * filename, int raw)
{
	int ret = -1;
	Asm * a;
	AsmCode * code;

	if((a = asm_new(gdeasm->arch, gdeasm->format)) == NULL)
		return -1;
	if((code = asm_open_deassemble(a, filename, raw)) != NULL)
		ret = _open_code(gdeasm, code);
	asm_close(a);
	asm_delete(a);
	return ret;
}

static int _open_code(GDeasm * gdeasm, AsmCode * code)
{
	int ret;
	AsmSection * sections;
	size_t sections_cnt;
	size_t i;

	asmcode_get_sections(code, &sections, &sections_cnt);
	for(i = 0; i < sections_cnt; i++)
		if((ret = _open_code_section(gdeasm, code, &sections[i])) != 0)
			break;
	return ret;
}

static int _open_code_section(GDeasm * gdeasm, AsmCode * code,
		AsmSection * section)
{
	GtkTreeIter iter;
	ArchInstructionCall * calls = NULL;
	size_t calls_cnt = 0;
	size_t i;

	gtk_tree_store_append(gdeasm->store, &iter, NULL);
	gtk_tree_store_set(gdeasm->store, &iter, 0, section->name, -1);
	if(asmcode_decode_section(code, section, &calls, &calls_cnt) != 0)
		return -1;
	for(i = 0; i < calls_cnt; i++)
		_open_instruction(gdeasm, &iter, &calls[i]);
	free(calls);
	return 0;
}

static void _open_instruction(GDeasm * gdeasm, GtkTreeIter * parent,
		ArchInstructionCall * call)
{
	GtkTreeIter iter;
	char buf[32];
	size_t i;
	ArchOperand * ao;
	char const * name;

	gtk_tree_store_append(gdeasm->store, &iter, parent);
	snprintf(buf, sizeof(buf), "%08lx", call->base);
	gtk_tree_store_set(gdeasm->store, &iter, 0, buf, 1, call->name, -1);
	for(i = 0; i < call->operands_cnt; i++)
	{
		ao = &call->operands[i];
		switch(AO_GET_TYPE(ao->definition))
		{
			case AOT_DREGISTER:
				_open_parse_dregister(buf, sizeof(buf), ao);
				break;
			case AOT_DREGISTER2:
				_open_parse_dregister2(buf, sizeof(buf), ao);
				break;
			case AOT_IMMEDIATE:
				_open_parse_immediate(buf, sizeof(buf), ao);
				break;
			case AOT_REGISTER:
				name = call->operands[i].value._register.name;
				snprintf(buf, sizeof(buf), "%%%s", name);
				break;
			default:
				buf[0] = '\0';
				break;
		}
		gtk_tree_store_set(gdeasm->store, &iter, i + 2, buf, -1);
	}
}

static void _open_parse_dregister(char * buf, size_t size, ArchOperand * ao)
{
	char const * name;

	name = ao->value.dregister.name;
	if(ao->value.dregister.offset == 0)
		snprintf(buf, size, "[%%%s]", name);
	else
		snprintf(buf, size, "[%%%s + $0x%lx]", name,
				(unsigned long)ao->value.dregister.offset);
}

static void _open_parse_dregister2(char * buf, size_t size, ArchOperand * ao)
{
	snprintf(buf, size, "[%%%s + %%%s]", ao->value.dregister2.name,
			ao->value.dregister2.name2);
}

static void _open_parse_immediate(char * buf, size_t size, ArchOperand * ao)
{
	snprintf(buf, size, "%s$0x%lx", ao->value.immediate.negative
			? "-" : "", (unsigned long)ao->value.immediate.value);
}


/* callbacks */
/* gdeasm_on_closex */
static gboolean _gdeasm_on_closex(void)
{
	gtk_main_quit();
	return TRUE;
}


/* gdeasm_on_open */
static void _gdeasm_on_open(gpointer data)
{
	GDeasm * gdeasm = data;
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
	gtk_tree_store_clear(gdeasm->store);
	_gdeasm_open(gdeasm, filename, 0);
	g_free(filename);
}


/* usage */
static int _usage(void)
{
	fputs("Usage: gdeasm filename\n", stderr);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	int o;
	GDeasm * gdeasm;
	char const * arch = NULL;
	char const * format = NULL;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "a:f:")) != -1)
		switch(o)
		{
			case 'a':
				arch = optarg;
				break;
			case 'f':
				format = optarg;
				break;
			default:
				return _usage();
		}
	if(optind != argc && optind + 1 != argc)
		return _usage();
	if((gdeasm = _gdeasm_new(arch, format, argv[optind])) == NULL)
		return 2;
	gtk_main();
	_gdeasm_delete(gdeasm);
	return 0;
}
