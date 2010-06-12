/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Phone */
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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <System.h>
#include "Phone.h"


/* Engineering */
/* private */
/* types */
typedef enum _EngineeringNeighborCellInformation
{
	ENCI_ARFCN = 0,
	ENCI_C1,
	ENCI_C2,
	ENCI_RXLEV,
	ENCI_BSIC,
	ENCI_CELL_ID,
	ENCI_LAC,
	ENCI_FRAME_OFFSET,
	ENCI_TIME_ALIGNMENT,
	ENCI_CBA,
	ENCI_CBQ,
	ENCI_CELL_TYPE_IND,
	ENCI_RAC,
	ENCI_CELL_RESEL_OFFSET,
	ENCI_TEMP_OFFSET,
	ENCI_RXLEV_ACC_MIN
} EngineeringNeighborCellInformation;

typedef struct _Engineering
{
	PhonePluginHelper * helper;
	guint source;

	EngineeringNeighborCellInformation enci;
	size_t enci_cnt;

	/* widgets */
	GtkWidget * window;
	GtkToolItem * play;
	GtkListStore * store;
	GtkWidget * view;
} Engineering;


/* constants */
enum { COL_FREQUENCY, COL_C1, COL_C2, COL_RXLEV, COL_BSIC, COL_CELL_ID, COL_LAC,
	COL_FRAME_OFFSET, COL_CELL_TYPE_IND };
#define COL_LAST COL_CELL_TYPE_IND
#define COL_COUNT (COL_LAST + 1)

static struct
{
	int col;
	char const * title;
} _engineering_columns[COL_COUNT + 1] =
{
	{ COL_FREQUENCY, "Frequency" },
	{ COL_C1, "C1" },
	{ COL_C2, "C2" },
	{ COL_RXLEV, "RX level" },
	{ COL_BSIC, "Station ID" },
	{ COL_CELL_ID, "Cell ID" },
	{ COL_LAC, "Area code" },
	{ COL_FRAME_OFFSET, "Frame offset" },
	{ COL_CELL_TYPE_IND, "Cell type" },
	{ 0, NULL }
};


/* prototypes */
static int _engineering_init(PhonePlugin * plugin);
static int _engineering_destroy(PhonePlugin * plugin);

/* callbacks */
static gboolean _on_engineering_closex(gpointer data);
static void _on_engineering_play_toggled(gpointer data);
static gboolean _on_engineering_timeout(gpointer data);
static int _on_engineering_trigger_em(PhonePlugin * plugin,
		char const * result);


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	"Engineering",
	NULL,
	_engineering_init,
	_engineering_destroy,
	NULL,
	NULL,
	NULL
};


/* private */
/* functions */
/* engineering_init */
static int _engineering_init(PhonePlugin * plugin)
{
	Engineering * engineering;
	GtkWidget * vbox;
	GtkWidget * toolbar;
	GtkWidget * scrolled;
	size_t i;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;

	if((engineering = malloc(sizeof(*engineering))) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	plugin->priv = engineering;
	engineering->helper = plugin->helper;
	engineering->source = 0;
	engineering->enci = 0;
	engineering->enci_cnt = 0;
	/* widgets */
	/* window */
	engineering->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(engineering->window), 200, 300);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(engineering->window),
			"stock_compile");
#endif
	gtk_window_set_title(GTK_WINDOW(engineering->window),
			"Engineering mode");
	g_signal_connect_swapped(G_OBJECT(engineering->window), "delete-event",
			G_CALLBACK(_on_engineering_closex), engineering);
	vbox = gtk_vbox_new(FALSE, 0);
	/* toolbar */
	toolbar = gtk_toolbar_new();
	engineering->play = gtk_toggle_tool_button_new_from_stock(
			GTK_STOCK_MEDIA_PLAY);
	g_signal_connect_swapped(G_OBJECT(engineering->play), "toggled",
			G_CALLBACK(_on_engineering_play_toggled), engineering);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), engineering->play, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, TRUE, 0);
	/* store */
	engineering->store = gtk_list_store_new(COL_COUNT,
			G_TYPE_STRING,		/* COL_FREQUENCY */
			G_TYPE_STRING,		/* COL_C1 */
			G_TYPE_STRING,		/* COL_C2 */
			G_TYPE_STRING,		/* COL_RXLEV */
			G_TYPE_STRING,		/* COL_BSIC */
			G_TYPE_STRING,		/* COL_CELL_ID */
			G_TYPE_STRING,		/* COL_LAC */
			G_TYPE_STRING,		/* COL_FRAME_OFFSET */
			G_TYPE_STRING);		/* COL_CELL_TYPE_IND */
	/* view */
	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	engineering->view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				engineering->store));
	/* columns */
	for(i = 0; _engineering_columns[i].title != NULL; i++)
	{
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(
				_engineering_columns[i].title, renderer, "text",
				_engineering_columns[i].col, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(engineering->view),
				column);
	}
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled),
			engineering->view);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(engineering->window), vbox);
	gtk_widget_show_all(engineering->window);
	/* trigger */
	plugin->helper->register_trigger(plugin->helper->phone, plugin, "%EM",
			_on_engineering_trigger_em);
	return 0;
}


/* engineering_destroy */
static int _engineering_destroy(PhonePlugin * plugin)
{
	Engineering * engineering = plugin->priv;

	if(engineering->source != 0)
		g_source_remove(engineering->source);
	gtk_widget_destroy(engineering->window);
	free(engineering);
	return 0;
}


/* callbacks */
/* on_engineering_closex */
static gboolean _on_engineering_closex(gpointer data)
{
	Engineering * engineering = data;

	gtk_widget_hide(engineering->window);
	/* FIXME unload the plugin */
	return TRUE;
}


/* on_engineering_play_toggled */
static void _on_engineering_play_toggled(gpointer data)
{
	Engineering * engineering = data;

	if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(
					engineering->play)))
		engineering->source = _on_engineering_timeout(engineering);
	else if(engineering->source != 0)
	{
		g_source_remove(engineering->source);
		engineering->source = 0;
	}
}


/* on_engineering_timeout */
static gboolean _on_engineering_timeout(gpointer data)
{
	Engineering * engineering = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	engineering->source = 0;
	engineering->helper->queue(engineering->helper->phone, "AT%EM=2,3");
	return FALSE;
}


/* on_engineering_trigger_em */
static int _trigger_em_do(Engineering * engineering, char const * buf,
		GtkTreeIter * iter);
static int _do_arfcn(Engineering * engineering, unsigned int arfcn,
		GtkTreeIter * iter);
static int _do_c1(Engineering * engineering, unsigned int c1,
		GtkTreeIter * iter);
static int _do_c2(Engineering * engineering, unsigned int c2,
		GtkTreeIter * iter);
static int _do_rxlev(Engineering * engineering, unsigned int rxlev,
		GtkTreeIter * iter);
static int _do_bsic(Engineering * engineering, unsigned int bsic,
		GtkTreeIter * iter);
static int _do_cell_id(Engineering * engineering, unsigned int cell_id,
		GtkTreeIter * iter);
static int _do_lac(Engineering * engineering, unsigned int lac,
		GtkTreeIter * iter);
static int _do_frame_offset(Engineering * engineering,
		unsigned int frame_offset, GtkTreeIter * iter);
static int _do_time_alignment(Engineering * engineering,
		unsigned int time_alignment);
static int _do_cba(Engineering * engineering, unsigned int cba);
static int _do_cbq(Engineering * engineering, unsigned int cbq);
static int _do_cell_type_ind(Engineering * engineering,
		unsigned int cell_type_ind, GtkTreeIter * iter);
static int _do_rac(Engineering * engineering, unsigned int rac);
static int _do_cell_resel_offset(Engineering * engineering,
		unsigned int cell_resel_offset);
static int _do_temp_offset(Engineering * engineering, unsigned int temp_offset);
static int _do_rxlev_acc_min(Engineering * engineering,
		unsigned int rxlev_acc_min);

static int _on_engineering_trigger_em(PhonePlugin * plugin, char const * result)
{
	Engineering * engineering = plugin->priv;
	unsigned int u;
	size_t i;
	size_t j;
	gboolean valid;
	GtkTreeIter iter;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(engineering->source == 0)
		engineering->source = g_timeout_add(5000,
				_on_engineering_timeout, engineering);
	if(sscanf(result, "%%EM: %u", &u) == 1)
	{
		engineering->enci = 0;
		engineering->enci_cnt = u;
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s(\"%s\") => %u\n", __func__, result,
				u);
#endif
		return 0;
	}
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(
				engineering->store), &iter);
	for(i = 0; i < engineering->enci_cnt; i++)
	{
		if(valid == FALSE)
			gtk_list_store_append(engineering->store, &iter);
		for(j = 0; result[j] != ',' && result[j] != '\0'; j++);
		_trigger_em_do(engineering, result, &iter);
		if(result[j] == '\0')
			break; /* XXX report when did not parse as many cells */
		result += j + 1;
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(
					engineering->store), &iter);
	}
	/* FIXME remove the following lines */
	engineering->enci++;
	return 0;
}

static int _trigger_em_do(Engineering * engineering, char const * buf,
		GtkTreeIter * iter)
{
	unsigned int u;

	if(sscanf(buf, "%u", &u) != 1)
		return 1;
	switch(engineering->enci)
	{
		case ENCI_ARFCN:
			return _do_arfcn(engineering, u, iter);
		case ENCI_C1:
			return _do_c1(engineering, u, iter);
		case ENCI_C2:
			return _do_c2(engineering, u, iter);
		case ENCI_RXLEV:
			return _do_rxlev(engineering, u, iter);
		case ENCI_BSIC:
			return _do_bsic(engineering, u, iter);
		case ENCI_CELL_ID:
			return _do_cell_id(engineering, u, iter);
		case ENCI_LAC:
			return _do_lac(engineering, u, iter);
		case ENCI_FRAME_OFFSET:
			return _do_frame_offset(engineering, u, iter);
		case ENCI_TIME_ALIGNMENT:
			return _do_time_alignment(engineering, u);
		case ENCI_CBA:
			return _do_cba(engineering, u);
		case ENCI_CBQ:
			return _do_cbq(engineering, u);
		case ENCI_CELL_TYPE_IND:
			return _do_cell_type_ind(engineering, u, iter);
		case ENCI_RAC:
			return _do_rac(engineering, u);
		case ENCI_CELL_RESEL_OFFSET:
			return _do_cell_resel_offset(engineering, u);
		case ENCI_TEMP_OFFSET:
			return _do_temp_offset(engineering, u);
		case ENCI_RXLEV_ACC_MIN:
			return _do_rxlev_acc_min(engineering, u);
	}
	return 1;
}

static int _do_arfcn(Engineering * engineering, unsigned int arfcn,
		GtkTreeIter * iter)
{
	double freq;
	char buf[32];

	if(arfcn == 0)
		freq = 0 / 0.0;
	else if(arfcn < 124) /* 900 MHz */
		freq = 935 + (arfcn - 511) * 0.2;
	else if(arfcn <= 172)
		freq = 0 / 0.0;
	else if(arfcn < 252) /* 850 MHz */
		freq = 869 + (arfcn - 127) * 0.2;
	else if(arfcn <= 511)
		freq = 0 / 0.0;
	else if(arfcn < 811) /* 1900 MHz */
		freq = 1930 + (arfcn - 511) * 0.2;
	else if(arfcn <= 974)
		freq = 0 / 0.0;
	else if(arfcn < 1023) /* 900 MHz */
		freq = 925 + (arfcn - 974) * 0.2;
	else
		freq = 0 / 0.0;
	snprintf(buf, sizeof(buf), "%.1lf", freq);
	gtk_list_store_set(engineering->store, iter, 0, buf, -1);
	return 0;
}

static int _do_c1(Engineering * engineering, unsigned int c1,
		GtkTreeIter * iter)
{
	char buf[32];

	snprintf(buf, sizeof(buf), "%u", c1);
	gtk_list_store_set(engineering->store, iter, COL_C1, buf, -1);
	return 0;
}

static int _do_c2(Engineering * engineering, unsigned int c2,
		GtkTreeIter * iter)
{
	char buf[32];

	snprintf(buf, sizeof(buf), "%u", c2);
	gtk_list_store_set(engineering->store, iter, COL_C2, buf, -1);
	return 0;
}

static int _do_rxlev(Engineering * engineering, unsigned int rxlev,
		GtkTreeIter * iter)
{
	char buf[32];

	/* FIXME implement properly */
	snprintf(buf, sizeof(buf), "%u", (rxlev / 2) + 2);
	gtk_list_store_set(engineering->store, iter, COL_RXLEV, buf, -1);
	return 0;
}

static int _do_bsic(Engineering * engineering, unsigned int bsic,
		GtkTreeIter * iter)
{
	char buf[32];

	snprintf(buf, sizeof(buf), "%u", bsic);
	gtk_list_store_set(engineering->store, iter, COL_BSIC, buf, -1);
	return 0;
}

static int _do_cell_id(Engineering * engineering, unsigned int cell_id,
		GtkTreeIter * iter)
{
	char buf[32];

	snprintf(buf, sizeof(buf), "%u", cell_id);
	gtk_list_store_set(engineering->store, iter, COL_CELL_ID, buf, -1);
	return 0;
}

static int _do_lac(Engineering * engineering, unsigned int lac,
		GtkTreeIter * iter)
{
	char buf[32];

	snprintf(buf, sizeof(buf), "%u", lac);
	gtk_list_store_set(engineering->store, iter, COL_LAC, buf, -1);
	return 0;
}

static int _do_frame_offset(Engineering * engineering,
		unsigned int frame_offset, GtkTreeIter * iter)
{
	char buf[32];

	snprintf(buf, sizeof(buf), "%u", frame_offset);
	gtk_list_store_set(engineering->store, iter, COL_FRAME_OFFSET, buf, -1);
	return 0;
}

static int _do_time_alignment(Engineering * engineering,
		unsigned int time_alignment)
{
	return 0;
}

static int _do_cba(Engineering * engineering, unsigned int cba)
{
	return 0;
}

static int _do_cbq(Engineering * engineering, unsigned int cbq)
{
	return 0;
}

static int _do_cell_type_ind(Engineering * engineering,
		unsigned int cell_type_ind, GtkTreeIter * iter)
{
	char const * type = "NA";

	switch(cell_type_ind)
	{
		case 1:
			type = "GSM";
			break;
		case 2:
			type = "GPRS";
			break;
	}
	gtk_list_store_set(engineering->store, iter, COL_CELL_TYPE_IND, type,
			-1);
	return 0;
}

static int _do_rac(Engineering * engineering, unsigned int rac)
{
	return 0;
}

static int _do_cell_resel_offset(Engineering * engineering,
		unsigned int cell_resel_offset)
{
	return 0;
}

static int _do_temp_offset(Engineering * engineering, unsigned int temp_offset)
{
	return 0;
}

static int _do_rxlev_acc_min(Engineering * engineering,
		unsigned int rxlev_acc_min)
{
	return 0;
}
