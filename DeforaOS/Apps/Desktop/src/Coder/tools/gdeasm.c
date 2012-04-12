/* $Id$ */
static char const _gdeasm_copyright[] =
"Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org>";
static char const _gdeasm_license[] =
"Redistribution and use in source and binary forms, with or without\n"
"modification, are permitted provided that the following conditions\n"
"are met:\n"
"1. Redistributions of source code must retain the above copyright\n"
"   notice, this list of conditions and the following disclaimer.\n"
"2. Redistributions in binary form must reproduce the above copyright\n"
"   notice, this list of conditions and the following disclaimer in the\n"
"   documentation and/or other materials provided with the distribution.\n"
"3. Neither the name of the authors nor the names of the contributors\n"
"   may be used to endorse or promote products derived from this software\n"
"   without specific prior written permission.\n"
"THIS SOFTWARE IS PROVIDED BY ITS AUTHORS AND CONTRIBUTORS ``AS IS'' AND\n"
"ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n"
"IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\n"
"ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE\n"
"FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\n"
"DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS\n"
"OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\n"
"HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT\n"
"LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY\n"
"OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF\n"
"SUCH DAMAGE.";
/* TODO:
 * - load comments as well
 * - add a preferences structure
 * - complete the function list
 * - add a window automatically displaying integers in base 2, 8, 10 and 16 */



#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <System.h>
#include <Devel/Asm.h>
#include <Desktop.h>
#include "../config.h"


/* GDeasm */
/* private */
/* types */
typedef enum _GDeasmAsmColumn
{
	GAC_ADDRESS = 0, GAC_NAME, GAC_OPERAND1, GAC_OPERAND2, GAC_OPERAND3,
	GAC_OPERAND4, GAC_OPERAND5, GAC_COMMENT, GAC_OFFSET, GAC_BASE
} GDeasmAsmColumn;
#define GAC_LAST GAC_BASE
#define GAC_COUNT (GAC_LAST + 1)

typedef enum _GDeasmFuncColumn
{
	GFC_NAME = 0, GFC_OFFSET_DISPLAY, GFC_OFFSET
} GDeasmFuncColumn;
#define GFC_LAST GFC_OFFSET
#define GFC_COUNT (GFC_LAST + 1)

typedef enum _GDeasmStrColumn
{
	GSC_STRING = 0
} GDeasmStrColumn;
#define GSC_LAST GSC_STRING
#define GSC_COUNT (GSC_LAST + 1)

typedef struct _GDeasm
{
	char * arch;
	char * format;

	gboolean modified;

	/* widgets */
	GtkWidget * window;
	GtkListStore * func_store;
	GtkListStore * str_store;
	GtkTreeStore * asm_store;
	GtkWidget * asm_view;
} GDeasm;


/* prototypes */
static GDeasm * _gdeasm_new(char const * arch, char const * format);
static void _gdeasm_delete(GDeasm * gdeasm);

/* useful */
static int _gdeasm_confirm(GDeasm * gdeasm, char const * message, ...);
static int _gdeasm_error(GDeasm * gdeasm, char const * message, int ret);
static int _gdeasm_open(GDeasm * gdeasm, char const * filename, int raw);
static int _gdeasm_save_comments(GDeasm * gdeasm, char const * filename);
static int _gdeasm_save_comments_dialog(GDeasm * gdeasm);

/* callbacks */
static void _gdeasm_on_about(gpointer data);
static void _gdeasm_on_close(gpointer data);
static gboolean _gdeasm_on_closex(gpointer data);
static void _gdeasm_on_comment_edited(GtkCellRendererText * renderer,
		gchar * arg1, gchar * arg2, gpointer data);
static void _gdeasm_on_function_activated(GtkTreeView * view,
		GtkTreePath * path, GtkTreeViewColumn * column, gpointer data);
static void _gdeasm_on_open(gpointer data);
static void _gdeasm_on_save_comments(gpointer data);

static int _usage(void);


/* constants */
static char const * _gdeasm_authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};

static DesktopMenu const _gdeasm_menu_file[] =
{
	{ "_Open...", G_CALLBACK(_gdeasm_on_open), GTK_STOCK_OPEN,
		GDK_CONTROL_MASK, GDK_KEY_O },
	{ "", NULL, NULL, 0, 0 },
	{ "_Save comments as...", G_CALLBACK(_gdeasm_on_save_comments), NULL,
		GDK_CONTROL_MASK, GDK_KEY_S },
	{ "", NULL, NULL, 0, 0 },
	{ "_Close", G_CALLBACK(_gdeasm_on_close), GTK_STOCK_CLOSE,
		GDK_CONTROL_MASK, GDK_KEY_W },
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenu const _gdeasm_menu_help[] =
{
#if GTK_CHECK_VERSION(2, 6, 0)
	{ "_About", G_CALLBACK(_gdeasm_on_about), GTK_STOCK_ABOUT, 0, 0 },
#else
	{ "_About", G_CALLBACK(_gdeasm_on_about), NULL, 0, 0 },
#endif
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenubar const _gdeasm_menubar[] =
{
	{ "_File", _gdeasm_menu_file },
	{ "_Help", _gdeasm_menu_help },
	{ NULL, NULL },
};

/* toolbar */
static DesktopToolbar _gdeasm_toolbar[] =
{
	{ "Open file", G_CALLBACK(_gdeasm_on_open), GTK_STOCK_OPEN, 0, 0,
		NULL },
	{ "", NULL, NULL, 0, 0, NULL },
	{ "Save comments", G_CALLBACK(_gdeasm_on_save_comments),
		GTK_STOCK_SAVE_AS, 0, 0, NULL },
	{ NULL, NULL, NULL, 0, 0, NULL }
};


/* functions */
/* gdeasm_new */
static GDeasm * _gdeasm_new(char const * arch, char const * format)
{
	GDeasm * gdeasm;
	GtkAccelGroup * accel;
	GtkWidget * vbox;
	GtkWidget * menubar;
	GtkWidget * toolbar;
	GtkWidget * hpaned;
	GtkWidget * vpaned;
	GtkWidget * scrolled;
	GtkWidget * treeview;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;
	char const * headers1[GFC_COUNT - 1] = { "Functions", "Offset" };
	char const * headers2[GSC_COUNT] = { "Strings" };
	char const * headers3[] = { "Address", "Instruction", "Operand",
		"Operand", "Operand", "Operand", "Operand", "Comment" };
	size_t i;

	if((gdeasm = malloc(sizeof(*gdeasm))) == NULL)
		return NULL;
	gdeasm->arch = (arch != NULL) ? strdup(arch) : NULL;
	gdeasm->format = (format != NULL) ? strdup(format) : NULL;
	gdeasm->modified = FALSE;
	/* widgets */
	gdeasm->func_store = gtk_list_store_new(GFC_COUNT, G_TYPE_STRING,
			G_TYPE_STRING, G_TYPE_INT);
	gdeasm->str_store = gtk_list_store_new(GSC_COUNT, G_TYPE_STRING);
	gdeasm->asm_store = gtk_tree_store_new(GAC_COUNT, G_TYPE_STRING,
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
			G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT);
	accel = gtk_accel_group_new();
	gdeasm->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_add_accel_group(GTK_WINDOW(gdeasm->window), accel);
	gtk_window_set_default_size(GTK_WINDOW(gdeasm->window), 640, 480);
	g_signal_connect_swapped(gdeasm->window, "delete-event", G_CALLBACK(
				_gdeasm_on_closex), gdeasm);
	vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
	menubar = desktop_menubar_create(_gdeasm_menubar, gdeasm, accel);
	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, TRUE, 0);
	/* toolbar */
	toolbar = desktop_toolbar_create(_gdeasm_toolbar, gdeasm, accel);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, TRUE, 0);
	/* view */
	hpaned = gtk_hpaned_new();
	vpaned = gtk_vpaned_new();
	/* functions */
	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				gdeasm->func_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview), TRUE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(treeview), FALSE);
	g_signal_connect(treeview, "row-activated", G_CALLBACK(
				_gdeasm_on_function_activated), gdeasm);
	for(i = 0; i < sizeof(headers1) / sizeof(*headers1); i++)
	{
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(headers1[i],
				renderer, "text", i, NULL);
		if(i == 1)
			g_object_set(renderer, "family", "Monospace", NULL);
		gtk_tree_view_column_set_sort_column_id(column, i);
		gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	}
	gtk_container_add(GTK_CONTAINER(scrolled), treeview);
	gtk_paned_pack1(GTK_PANED(vpaned), scrolled, FALSE, TRUE);
	/* strings */
	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				gdeasm->str_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview), TRUE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(treeview), FALSE);
	for(i = 0; i < sizeof(headers2) / sizeof(*headers2); i++)
	{
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(headers2[i],
				renderer, "text", i, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	}
	gtk_container_add(GTK_CONTAINER(scrolled), treeview);
	gtk_paned_pack2(GTK_PANED(vpaned), scrolled, FALSE, FALSE);
	gtk_paned_add1(GTK_PANED(hpaned), vpaned);
	/* assembly */
	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				gdeasm->asm_store));
	gdeasm->asm_view = treeview;
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview), FALSE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(treeview), FALSE);
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(treeview), TRUE);
	for(i = 0; i < sizeof(headers3) / sizeof(*headers3); i++)
	{
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(headers3[i],
				renderer, "text", i, NULL);
		if(i == 0)
			g_object_set(renderer, "family", "Monospace", NULL);
		else if(i == 7) /* the last column is editable */
		{
			g_object_set(renderer, "editable", TRUE, "style",
					PANGO_STYLE_ITALIC, NULL);
			g_signal_connect(renderer, "edited", G_CALLBACK(
						_gdeasm_on_comment_edited),
					gdeasm);
		}
		gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	}
	gtk_container_add(GTK_CONTAINER(scrolled), treeview);
	gtk_paned_add2(GTK_PANED(hpaned), scrolled);
	gtk_box_pack_start(GTK_BOX(vbox), hpaned, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(gdeasm->window), vbox);
	gtk_widget_show_all(gdeasm->window);
	return gdeasm;
}


/* gdeasm_delete */
static void _gdeasm_delete(GDeasm * gdeasm)
{
	free(gdeasm->arch);
	free(gdeasm->format);
	free(gdeasm);
}


/* useful */
/* gdeasm_confirm */
static int _gdeasm_confirm(GDeasm * gdeasm, char const * message, ...)
{
	GtkWidget * dialog;
	va_list ap;
	char const * action;
	int res;

	dialog = gtk_message_dialog_new(GTK_WINDOW(gdeasm->window),
			GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_NONE,
# if GTK_CHECK_VERSION(2, 6, 0)
			"%s", "Question");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
# endif
			"%s", message);
	va_start(ap, message);
	while((action = va_arg(ap, char const *)) != NULL)
		gtk_dialog_add_button(GTK_DIALOG(dialog),
				action, va_arg(ap, int));
	va_end(ap);
	gtk_window_set_title(GTK_WINDOW(dialog), "Question");
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return res;
}


/* gdeasm_error */
static int _gdeasm_error(GDeasm * gdeasm, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(gdeasm->window),
			GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
# if GTK_CHECK_VERSION(2, 6, 0)
			"%s", "Error");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
# endif
			"%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret;
}


/* gdeasm_open */
static int _open_code(GDeasm * gdeasm, AsmCode * af);
static int _open_code_section(GDeasm * gdeasm, AsmCode * code,
		AsmSection * section);
static void _open_functions(GDeasm * gdeasm, AsmFunction * af, size_t af_cnt);
static void _open_instruction(GDeasm * gdeasm, GtkTreeIter * parent,
		ArchInstructionCall * call);
static void _open_parse_dregister(char * buf, size_t size, ArchOperand * ao);
static void _open_parse_dregister2(char * buf, size_t size, ArchOperand * ao);
static void _open_parse_immediate(char * buf, size_t size, ArchOperand * ao);
static void _open_strings(GDeasm * gdeasm, AsmString * as, size_t as_cnt);

static int _gdeasm_open(GDeasm * gdeasm, char const * filename, int raw)
{
	int ret = -1;
	int res;
	Asm * a;
	AsmCode * code;
	AsmFunction * af;
	size_t af_cnt;
	AsmString * as;
	size_t as_cnt;

	if(gdeasm->modified != FALSE)
	{
		res = _gdeasm_confirm(gdeasm, "There are unsaved comments.\n"
				"Discard or save them?",
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
#if GTK_CHECK_VERSION(2, 12, 0)
				GTK_STOCK_DISCARD, GTK_RESPONSE_REJECT,
#else
				"Discard", GTK_RESPONSE_REJECT,
#endif
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
		if(res == GTK_RESPONSE_ACCEPT)
		{
			if(_gdeasm_save_comments_dialog(gdeasm) != 0)
				return 0;
		}
		else if(res != GTK_RESPONSE_REJECT)
			return 0;
	}
	if((a = asm_new(gdeasm->arch, gdeasm->format)) == NULL)
		return -_gdeasm_error(gdeasm, error_get(), 1);
	if((code = asm_open_deassemble(a, filename, raw)) != NULL)
	{
		gtk_list_store_clear(gdeasm->func_store);
		gtk_list_store_clear(gdeasm->str_store);
		gtk_tree_store_clear(gdeasm->asm_store);
		gdeasm->modified = FALSE;
		ret = _open_code(gdeasm, code);
		asmcode_get_functions(code, &af, &af_cnt);
		_open_functions(gdeasm, af, af_cnt);
		asmcode_get_strings(code, &as, &as_cnt);
		_open_strings(gdeasm, as, as_cnt);
		asm_close(a);
	}
	asm_delete(a);
	if(ret != 0)
		_gdeasm_error(gdeasm, error_get(), 1);
	return ret;
}

static int _open_code(GDeasm * gdeasm, AsmCode * code)
{
	int ret = 0;
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

	gtk_tree_store_append(gdeasm->asm_store, &iter, NULL);
	gtk_tree_store_set(gdeasm->asm_store, &iter, 1, section->name, -1);
	if(asmcode_decode_section(code, section, &calls, &calls_cnt) != 0)
		return -1;
	for(i = 0; i < calls_cnt; i++)
		_open_instruction(gdeasm, &iter, &calls[i]);
	free(calls);
	return 0;
}

static void _open_functions(GDeasm * gdeasm, AsmFunction * af, size_t af_cnt)
{
	size_t i;
	GtkTreeIter iter;
	char buf[16];

	for(i = 0; i < af_cnt; i++)
	{
		if(af[i].offset >= 0)
			snprintf(buf, sizeof(buf), "%08lx", af[i].offset);
		else
			buf[0] = '\0';
		gtk_list_store_append(gdeasm->func_store, &iter);
		gtk_list_store_set(gdeasm->func_store, &iter,
				GFC_NAME, af[i].name, GFC_OFFSET_DISPLAY, buf,
				GFC_OFFSET, af[i].offset, -1);
	}
}

static void _open_instruction(GDeasm * gdeasm, GtkTreeIter * parent,
		ArchInstructionCall * call)
{
	GtkTreeIter iter;
	char buf[32];
	size_t i;
	ArchOperand * ao;
	char const * name;

	gtk_tree_store_append(gdeasm->asm_store, &iter, parent);
	snprintf(buf, sizeof(buf), "%08lx", call->base);
	gtk_tree_store_set(gdeasm->asm_store, &iter, GAC_ADDRESS, buf,
			GAC_NAME, call->name, GAC_OFFSET, call->offset,
			GAC_BASE, call->base, -1);
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
				if(AO_GET_VALUE(ao->definition)
						== AOI_REFERS_STRING
						|| AO_GET_VALUE(ao->definition)
						== AOI_REFERS_FUNCTION)
					gtk_tree_store_set(gdeasm->asm_store,
							&iter, GAC_COMMENT,
							ao->value.immediate.name,
							-1);
				break;
			case AOT_REGISTER:
				name = call->operands[i].value._register.name;
				snprintf(buf, sizeof(buf), "%%%s", name);
				break;
			default:
				buf[0] = '\0';
				break;
		}
		gtk_tree_store_set(gdeasm->asm_store, &iter, GAC_OPERAND1 + i,
				buf, -1);
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

static void _open_strings(GDeasm * gdeasm, AsmString * as, size_t as_cnt)
{
	size_t i;
	GtkTreeIter iter;

	for(i = 0; i < as_cnt; i++)
	{
		gtk_list_store_append(gdeasm->str_store, &iter);
		gtk_list_store_set(gdeasm->str_store, &iter,
				GSC_STRING, as[i].name, -1);
	}
}


/* gdeasm_save_comments */
struct _save_comments_foreach_args
{
	int ret;
	GDeasm * gdeasm;
	Config * config;
};
static gboolean _save_comments_foreach(GtkTreeModel * model, GtkTreePath * path,
		GtkTreeIter * iter, gpointer data);

static int _gdeasm_save_comments(GDeasm * gdeasm, char const * filename)
{
	struct _save_comments_foreach_args args;

	args.ret = 0;
	args.gdeasm = gdeasm;
	if((args.config = config_new()) == NULL)
		return -_gdeasm_error(gdeasm, error_get(), 1);
	gtk_tree_model_foreach(GTK_TREE_MODEL(gdeasm->asm_store),
			_save_comments_foreach, &args);
	if(args.ret == 0)
	{
		if(config_save(args.config, filename) == 0)
			gdeasm->modified = FALSE;
		else
			args.ret = -_gdeasm_error(gdeasm, error_get(), 1);
	}
	config_delete(args.config);
	return args.ret;
}

static gboolean _save_comments_foreach(GtkTreeModel * model, GtkTreePath * path,
		GtkTreeIter * iter, gpointer data)
{
	struct _save_comments_foreach_args * args = data;
	int offset;
	gchar * p;
	char buf[16];

	gtk_tree_model_get(model, iter, GAC_OFFSET, &offset, GAC_COMMENT, &p,
			-1);
	if(p != NULL && strlen(p) > 0)
	{
		snprintf(buf, sizeof(buf), "0x%x", offset);
		if(config_set(args->config, "comments", buf, p) != 0)
			args->ret = -_gdeasm_error(args->gdeasm, error_get(),
					1);
	}
	g_free(p);
	return (args->ret == 0) ? FALSE : TRUE;
}


/* gdeasm_save_comments_dialog */
static int _gdeasm_save_comments_dialog(GDeasm * gdeasm)
{
	int ret = -1;
	GtkWidget * dialog;
	GtkFileFilter * filter;
	char * filename = NULL;

	dialog = gtk_file_chooser_dialog_new("Save comments as...", NULL,
			GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
	filter = gtk_file_filter_new();
        gtk_file_filter_set_name(filter, "GDeasm files");
        gtk_file_filter_add_pattern(filter, "*.gdeasm");
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
        gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "All files");
	gtk_file_filter_add_pattern(filter, "*");
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename != NULL)
		ret = _gdeasm_save_comments(gdeasm, filename);
	g_free(filename);
	return ret;
}


/* callbacks */
/* gdeasm_on_about */
static void _gdeasm_on_about(gpointer data)
{
	GDeasm * gdeasm = data;
	GtkWidget * dialog;

	dialog = desktop_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(
				gdeasm->window));
	desktop_about_dialog_set_authors(dialog, _gdeasm_authors);
	desktop_about_dialog_set_copyright(dialog, _gdeasm_copyright);
	desktop_about_dialog_set_logo_icon_name(dialog,
			"applications-development");
	desktop_about_dialog_set_license(dialog, _gdeasm_license);
	desktop_about_dialog_set_name(dialog, "GDeasm");
	desktop_about_dialog_set_version(dialog, VERSION);
	desktop_about_dialog_set_website(dialog,
			"http://www.defora.org/");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}


/* gdeasm_on_close */
static void _gdeasm_on_close(gpointer data)
{
	GDeasm * gdeasm = data;
	int res;

	if(gdeasm->modified != FALSE)
	{
		res = _gdeasm_confirm(gdeasm, "There are unsaved comments.\n"
				"Discard or save them?",
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
				NULL);
		if(res == GTK_RESPONSE_ACCEPT)
		{
			if(_gdeasm_save_comments_dialog(gdeasm) != 0)
				return;
		}
		else if(res != GTK_RESPONSE_CLOSE)
			return;
	}
	gtk_widget_hide(gdeasm->window);
	gtk_main_quit();
}


/* gdeasm_on_closex */
static gboolean _gdeasm_on_closex(gpointer data)
{
	GDeasm * gdeasm = data;

	_gdeasm_on_close(gdeasm);
	return TRUE;
}


/* gdeasm_on_comment_edited */
static void _gdeasm_on_comment_edited(GtkCellRendererText * renderer,
		gchar * arg1, gchar * arg2, gpointer data)
{
	GDeasm * gdeasm = data;
	GtkTreeModel * model = GTK_TREE_MODEL(gdeasm->asm_store);
	GtkTreeIter iter;

	if(gtk_tree_model_get_iter_from_string(model, &iter, arg1) == TRUE)
	{
		gtk_tree_store_set(gdeasm->asm_store, &iter, 7, arg2, -1);
		gdeasm->modified = TRUE;
	}
}


/* gdeasm_on_function_activated */
static void _gdeasm_on_function_activated(GtkTreeView * view,
		GtkTreePath * path, GtkTreeViewColumn * column, gpointer data)
{
	GDeasm * gdeasm = data;
	GtkTreeModel * model = GTK_TREE_MODEL(gdeasm->func_store);
	GtkTreeIter iter;
	GtkTreeIter parent;
	gint offset;
	gboolean valid;
	gint u;
	GtkTreeSelection * treesel;

	if(gtk_tree_model_get_iter(model, &iter, path) != TRUE)
		return;
	gtk_tree_model_get(model, &iter, GFC_OFFSET, &offset, -1);
	if(offset < 0)
		return;
	model = GTK_TREE_MODEL(gdeasm->asm_store);
	for(valid = gtk_tree_model_get_iter_first(model, &parent); valid;
			valid = gtk_tree_model_iter_next(model, &parent))
		for(valid = gtk_tree_model_iter_children(model, &iter, &parent);
				valid;
				valid = gtk_tree_model_iter_next(model, &iter))
		{
			gtk_tree_model_get(model, &iter, GAC_BASE, &u, -1);
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() %x, %x\n", __func__,
					offset, u);
#endif
			if(offset != u)
				continue;
			view = GTK_TREE_VIEW(gdeasm->asm_view);
			treesel = gtk_tree_view_get_selection(view);
			gtk_tree_selection_select_iter(treesel, &iter);
			path = gtk_tree_model_get_path(model, &iter);
			gtk_tree_view_expand_to_path(view, path);
			gtk_tree_view_scroll_to_cell(view, path, NULL, FALSE,
					0.0, 0.0);
			gtk_tree_path_free(path);
			return;
		}
}


/* gdeasm_on_open */
static void _gdeasm_on_open(gpointer data)
{
	GDeasm * gdeasm = data;
	GtkWidget * dialog;
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkFileFilter * filter;
	char * filename = NULL;
	int raw;

	dialog = gtk_file_chooser_dialog_new("Open file...", NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	vbox = GTK_DIALOG(dialog)->vbox;
#endif
	widget = gtk_check_button_new_with_mnemonic("Open file in _raw mode");
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	gtk_widget_show_all(vbox);
	filter = gtk_file_filter_new();
        gtk_file_filter_set_name(filter, "Executable files");
        gtk_file_filter_add_mime_type(filter, "application/x-executable");
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
        gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);
	filter = gtk_file_filter_new();
        gtk_file_filter_set_name(filter, "Objects");
        gtk_file_filter_add_mime_type(filter, "application/x-object");
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "All files");
	gtk_file_filter_add_pattern(filter, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
		raw = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	}
	gtk_widget_destroy(dialog);
	if(filename != NULL)
		_gdeasm_open(gdeasm, filename, raw);
	g_free(filename);
}


/* gdeasm_on_save_comments */
static void _gdeasm_on_save_comments(gpointer data)
{
	GDeasm * gdeasm = data;

	_gdeasm_save_comments_dialog(gdeasm);
}


/* usage */
static int _usage(void)
{
	fputs("Usage: gdeasm [-D][-a arch][-f format] filename\n", stderr);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	int o;
	GDeasm * gdeasm;
	int raw = 0;
	char const * arch = NULL;
	char const * format = NULL;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "Da:f:")) != -1)
		switch(o)
		{
			case 'D':
				raw = 1;
				break;
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
	if((gdeasm = _gdeasm_new(arch, format)) == NULL)
		return 2;
	if(optind + 1 == argc)
		_gdeasm_open(gdeasm, argv[optind], raw);
	gtk_main();
	_gdeasm_delete(gdeasm);
	return 0;
}
