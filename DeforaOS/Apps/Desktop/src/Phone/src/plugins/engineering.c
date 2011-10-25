/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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
typedef enum _EngineeringLocationPagingParameters
{
	ELPP_BS_PA_MFRMS = 0,
	ELPP_T3212,
	ELPP_MCC,
	ELPP_MNC,
	ELPP_TMSI
} EngineeringLocationPagingParameter;

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

typedef enum _EngineeringServingCellInformation
{
	ESCI_ARFCN = 0,
	ESCI_C1,
	ESCI_C2,
	ESCI_RXLEV,
	ESCI_BSIC,
	ESCI_CELL_ID,
	ESCI_DSC,
	ESCI_TXLEV,
	ESCI_TN,
	ESCI_RLT,
	ESCI_TAV,
	ESCI_RXLEV_F,
	ESCI_RXLEV_S,
	ESCI_RXQUAL_F,
	ESCI_RXQUAL_S,
	ESCI_LAC,
	ESCI_CBA,
	ESCI_CBQ,
	ESCI_CELL_TYPE_IND,
	ESCI_VOCODER
} EngineeringServingCellInformation;

typedef struct _Engineering
{
	PhonePluginHelper * helper;
	guint source;

	EngineeringNeighborCellInformation enci;
	size_t enci_cnt;

	/* widgets */
	GtkWidget * window;
	GtkToolItem * tb_play;
	GtkToolItem * tb_fullscreen;
	GtkListStore * sc_store;
	GtkWidget * sc_view;
	GtkListStore * nc_store;
	GtkWidget * nc_view;
} Engineering;


/* constants */
/* serving cell */
enum {
	SC_COL_FREQUENCY, SC_COL_C1, SC_COL_C2, SC_COL_RX_LEVEL,
	SC_COL_STATION_ID, SC_COL_CELL_ID, SC_COL_TX_LEVEL,
	SC_COL_TIMESLOT_NUMBER, SC_COL_LAC, SC_COL_TMSI
};
#define SC_COL_LAST SC_COL_TMSI
#define SC_COL_COUNT (SC_COL_LAST + 1)

static struct
{
	int col;
	char const * title;
} _engineering_sc_columns[SC_COL_COUNT + 1] =
{
	{ SC_COL_FREQUENCY, "Frequency" },
	{ SC_COL_C1, "C1" },
	{ SC_COL_C2, "C2" },
	{ SC_COL_RX_LEVEL, "RX level" },
	{ SC_COL_STATION_ID, "Station ID" },
	{ SC_COL_CELL_ID, "Cell ID" },
	{ SC_COL_LAC, "Area code" },
	{ SC_COL_TX_LEVEL, "TX level" },
	{ SC_COL_TIMESLOT_NUMBER, "Timeslot" },
	{ SC_COL_TMSI, "TMSI" },
	{ 0, NULL }
};

/* neighbor cells */
enum { NC_COL_FREQUENCY, NC_COL_C1, NC_COL_C2, NC_COL_RXLEV, NC_COL_BSIC,
	NC_COL_CELL_ID, NC_COL_LAC, NC_COL_FRAME_OFFSET, NC_COL_CBA, NC_COL_CBQ,
	NC_COL_CELL_TYPE_IND, NC_COL_RAC };
#define NC_COL_LAST NC_COL_RAC
#define NC_COL_COUNT (NC_COL_LAST + 1)

static struct
{
	int col;
	char const * title;
} _engineering_nc_columns[NC_COL_COUNT + 1] =
{
	{ NC_COL_FREQUENCY, "Frequency" },
	{ NC_COL_C1, "C1" },
	{ NC_COL_C2, "C2" },
	{ NC_COL_RXLEV, "RX level" },
	{ NC_COL_BSIC, "Station ID" },
	{ NC_COL_CELL_ID, "Cell ID" },
	{ NC_COL_LAC, "Area code" },
	{ NC_COL_FRAME_OFFSET, "Frame offset" },
	{ NC_COL_CBA, "Cell Bar Access" },
	{ NC_COL_CBQ, "Cell Bar Qualify" },
	{ NC_COL_CELL_TYPE_IND, "Cell type" },
	{ NC_COL_RAC, "Routing area code" },
	{ 0, NULL }
};


/* prototypes */
static int _engineering_init(PhonePlugin * plugin);
static void _engineering_destroy(PhonePlugin * plugin);

static double _engineering_get_frequency(unsigned int arfcn);

/* callbacks */
static gboolean _on_engineering_closex(gpointer data);
static void _on_engineering_play_toggled(gpointer data);
static void _on_engineering_fullscreen_toggled(gpointer data);
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
	GtkWidget * paned;
	GtkWidget * frame;
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
	engineering->tb_play = gtk_toggle_tool_button_new_from_stock(
			GTK_STOCK_MEDIA_PLAY);
	g_signal_connect_swapped(G_OBJECT(engineering->tb_play), "toggled",
			G_CALLBACK(_on_engineering_play_toggled), engineering);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), engineering->tb_play, -1);
	engineering->tb_fullscreen = gtk_toggle_tool_button_new_from_stock(
			GTK_STOCK_FULLSCREEN);
	g_signal_connect_swapped(G_OBJECT(engineering->tb_fullscreen),
			"toggled", G_CALLBACK(
				_on_engineering_fullscreen_toggled),
			engineering);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), engineering->tb_fullscreen,
			-1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, TRUE, 0);
	/* serving cell view */
	paned = gtk_vpaned_new();
	frame = gtk_frame_new("Serving cell");
	engineering->sc_store = gtk_list_store_new(SC_COL_COUNT,
			G_TYPE_STRING,		/* SC_COL_FREQUENCY */
			G_TYPE_STRING,		/* SC_COL_C1 */
			G_TYPE_STRING,		/* SC_COL_C2 */
			G_TYPE_STRING,		/* SC_COL_RX_LEVEL */
			G_TYPE_STRING,		/* SC_COL_STATION_ID */
			G_TYPE_STRING,		/* SC_COL_CELL_ID */
			G_TYPE_STRING,		/* SC_COL_TX_LEVEL */
			G_TYPE_STRING,		/* SC_COL_TIMESLOT_NUMBER */
			G_TYPE_STRING,		/* SC_COL_LAC */
			G_TYPE_STRING);		/* SC_COL_TMSI */
	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	engineering->sc_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				engineering->sc_store));
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(engineering->sc_view), TRUE);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(
					engineering->sc_view)),
			GTK_SELECTION_NONE);
	/* columns */
	for(i = 0; _engineering_sc_columns[i].title != NULL; i++)
	{
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(
				_engineering_sc_columns[i].title, renderer,
				"text", _engineering_sc_columns[i].col, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(engineering->sc_view),
				column);
	}
	gtk_container_add(GTK_CONTAINER(scrolled), engineering->sc_view);
	gtk_container_add(GTK_CONTAINER(frame), scrolled);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_paned_add1(GTK_PANED(paned), frame);
	/* neighbor cells view */
	frame = gtk_frame_new("Neighbor cells");
	engineering->nc_store = gtk_list_store_new(NC_COL_COUNT,
			G_TYPE_STRING,		/* NC_COL_FREQUENCY */
			G_TYPE_STRING,		/* NC_COL_C1 */
			G_TYPE_STRING,		/* NC_COL_C2 */
			G_TYPE_STRING,		/* NC_COL_RXLEV */
			G_TYPE_STRING,		/* NC_COL_BSIC */
			G_TYPE_STRING,		/* NC_COL_CELL_ID */
			G_TYPE_STRING,		/* NC_COL_LAC */
			G_TYPE_STRING,		/* NC_COL_FRAME_OFFSET */
			G_TYPE_STRING,		/* NC_COL_CBA */
			G_TYPE_STRING,		/* NC_COL_CBQ */
			G_TYPE_STRING,		/* NC_COL_CELL_TYPE_IND */
			G_TYPE_STRING);		/* NC_COL_RAC */
	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	engineering->nc_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				engineering->nc_store));
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(engineering->nc_view), TRUE);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(
					engineering->nc_view)),
			GTK_SELECTION_NONE);
	/* columns */
	for(i = 0; _engineering_nc_columns[i].title != NULL; i++)
	{
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(
				_engineering_nc_columns[i].title, renderer,
				"text", _engineering_nc_columns[i].col, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(engineering->nc_view),
				column);
	}
	gtk_container_add(GTK_CONTAINER(scrolled), engineering->nc_view);
	gtk_container_add(GTK_CONTAINER(frame), scrolled);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	gtk_container_add(GTK_CONTAINER(engineering->window), vbox);
	gtk_paned_add2(GTK_PANED(paned), frame);
	gtk_box_pack_start(GTK_BOX(vbox), paned, TRUE, TRUE, 0);
	gtk_widget_show_all(engineering->window);
	/* trigger */
#if 0 /* FIXME reimplement using an extension to the Hayes modem plug-in */
	plugin->helper->register_trigger(plugin->helper->phone, plugin, "%EM",
			_on_engineering_trigger_em);
#endif
	return 0;
}


/* engineering_destroy */
static void _engineering_destroy(PhonePlugin * plugin)
{
	Engineering * engineering = plugin->priv;

	if(engineering->source != 0)
		g_source_remove(engineering->source);
	gtk_widget_destroy(engineering->window);
	free(engineering);
}


/* engineering_get_frequency */
static double _engineering_get_frequency(unsigned int arfcn)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u)\n", __func__, arfcn);
#endif
	if(arfcn == 0)
		return 0 / 0.0;
	else if(arfcn < 124) /* 900 MHz */
		return 935 + (arfcn - 511) * 0.2;
	else if(arfcn <= 172)
		return 0 / 0.0;
	else if(arfcn < 252) /* 850 MHz */
		return 869 + (arfcn - 127) * 0.2;
	else if(arfcn <= 511)
		return 0 / 0.0;
	/* XXX the values for the 1800 and 1900 MHz band overlap */
	else if(arfcn < 811) /* 1900 MHz */
		return 1930 + (arfcn - 511) * 0.2;
	else if(arfcn < 886) /* 1800 MHz */
		return 1805 + (arfcn - 511) * 0.2;
	else if(arfcn <= 974)
		return 0 / 0.0;
	else if(arfcn < 1023) /* 900 MHz */
		return 925 + (arfcn - 974) * 0.2;
	else
		return 0 / 0.0;
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


/* on_engineering_fullscreen_toggled */
static void _on_engineering_fullscreen_toggled(gpointer data)
{
	Engineering * engineering = data;

	if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(
					engineering->tb_fullscreen)))
		gtk_window_fullscreen(GTK_WINDOW(engineering->window));
	else
		gtk_window_unfullscreen(GTK_WINDOW(engineering->window));
}


/* on_engineering_play_toggled */
static void _on_engineering_play_toggled(gpointer data)
{
	Engineering * engineering = data;

	if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(
					engineering->tb_play)))
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
#if 0 /* FIXME implement again otherwise */
	engineering->helper->queue(engineering->helper->phone, "AT%EM=2,1");
	engineering->helper->queue(engineering->helper->phone, "AT%EM=2,2");
	engineering->helper->queue(engineering->helper->phone, "AT%EM=2,3");
	engineering->helper->queue(engineering->helper->phone, "AT%EM=2,4");
#endif
	return FALSE;
}


/* on_engineering_trigger_em */
static int _trigger_em1_do(Engineering * engineering, unsigned int * p,
		size_t cnt);
static int _trigger_em2_do(Engineering * engineering, unsigned int * p,
		size_t cnt);
static int _trigger_em3_do(Engineering * engineering, char const * buf,
		GtkTreeIter * iter);
static int _trigger_em4_do(Engineering * engineering, unsigned int * p,
		size_t cnt);
static int _do_arfcn(Engineering * engineering, unsigned int arfcn,
		GtkTreeIter * iter);
static int _do_c1(Engineering * engineering, int c1, GtkTreeIter * iter);
static int _do_c2(Engineering * engineering, int c2, GtkTreeIter * iter);
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
static int _do_cba(Engineering * engineering, unsigned int cba,
		GtkTreeIter * iter);
static int _do_cbq(Engineering * engineering, unsigned int cbq,
		GtkTreeIter * iter);
static int _do_cell_type_ind(Engineering * engineering,
		unsigned int cell_type_ind, GtkTreeIter * iter);
static int _do_rac(Engineering * engineering, unsigned int rac,
		GtkTreeIter * iter);
static int _do_cell_resel_offset(Engineering * engineering,
		unsigned int cell_resel_offset);
static int _do_temp_offset(Engineering * engineering, unsigned int temp_offset);
static int _do_rxlev_acc_min(Engineering * engineering,
		unsigned int rxlev_acc_min);

static int _on_engineering_trigger_em(PhonePlugin * plugin, char const * result)
{
	Engineering * engineering = plugin->priv;
	int res;
	unsigned int p[20];
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
	/* scan as many as we may expect */
	res = sscanf(result, "%%EM: %u,%u,%u,%u,%u"",%u,%u,%u,%u,%u"
			",%u,%u,%u,%u,%u"",%u,%u,%u,%u,%u",
			&p[0], &p[1], &p[2], &p[3], &p[4],
			&p[5], &p[6], &p[7], &p[8], &p[9],
			&p[10], &p[11], &p[12], &p[13], &p[14],
			&p[15], &p[16], &p[17], &p[18], &p[19]);
	if(res == 20)
		return _trigger_em1_do(engineering, p, 20);
	if(res == 11)
		return _trigger_em2_do(engineering, p, 11);
	if(res == 5)
		return _trigger_em4_do(engineering, p, 5);
	if(res == 1)
	{
		engineering->enci = 0;
		engineering->enci_cnt = p[0];
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s(\"%s\") => %u\n", __func__, result,
				p[0]);
#endif
		return 0;
	}
	if(res != 0)
		return 1;
	/* XXX assumes it's em3 */
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(
				engineering->nc_store), &iter);
	for(i = 0; i < engineering->enci_cnt; i++)
	{
		if(valid == FALSE)
			gtk_list_store_append(engineering->nc_store, &iter);
		for(j = 0; result[j] != ',' && result[j] != '\0'; j++);
		_trigger_em3_do(engineering, result, &iter);
		if(result[j] == '\0')
			break; /* XXX report when did not parse as many cells */
		result += j + 1;
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(
					engineering->nc_store), &iter);
	}
	if(valid && gtk_tree_model_iter_next(GTK_TREE_MODEL(
					engineering->nc_store), &iter))
		while(gtk_list_store_remove(engineering->nc_store, &iter));
	engineering->enci++;
	return 0;
}

static int _trigger_em1_do(Engineering * engineering, unsigned int * p,
		size_t cnt)
{
	EngineeringServingCellInformation esci;
	GtkTreeIter iter;
	size_t i;
	char buf[32];
	double freq;

	if(cnt != 20)
		return 1;
	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(engineering->sc_store),
				&iter) == FALSE)
		gtk_list_store_append(engineering->sc_store, &iter);
	for(i = 0; i < cnt; i++)
	{
		esci = i;
		snprintf(buf, sizeof(buf), "%u", p[i]);
		switch(esci)
		{
			case ESCI_ARFCN:
				freq = _engineering_get_frequency(p[i]);
				snprintf(buf, sizeof(buf), "%.1lf", freq);
				gtk_list_store_set(engineering->sc_store, &iter,
						SC_COL_FREQUENCY, buf, -1);
				break;
			case ESCI_C1:
				gtk_list_store_set(engineering->sc_store, &iter,
						SC_COL_C1, buf, -1);
				break;
			case ESCI_C2:
				gtk_list_store_set(engineering->sc_store, &iter,
						SC_COL_C2, buf, -1);
				break;
			case ESCI_RXLEV:
				snprintf(buf, sizeof(buf), "%u", (p[i] / 2)
						+ 2);
				gtk_list_store_set(engineering->sc_store, &iter,
						SC_COL_RX_LEVEL, buf, -1);
				break;
			case ESCI_BSIC:
				gtk_list_store_set(engineering->sc_store, &iter,
						SC_COL_STATION_ID, buf, -1);
				break;
			case ESCI_CELL_ID:
				gtk_list_store_set(engineering->sc_store, &iter,
						SC_COL_CELL_ID, buf, -1);
				break;
			case ESCI_TXLEV:
				gtk_list_store_set(engineering->sc_store, &iter,
						SC_COL_TX_LEVEL, buf, -1);
				break;
			case ESCI_TN:
				gtk_list_store_set(engineering->sc_store, &iter,
						SC_COL_TIMESLOT_NUMBER, buf,
						-1);
				break;
			case ESCI_LAC:
				gtk_list_store_set(engineering->sc_store, &iter,
						SC_COL_LAC, buf, -1);
				break;
			case ESCI_DSC:
			case ESCI_RLT:
			case ESCI_TAV:
			case ESCI_RXLEV_F:
			case ESCI_RXLEV_S:
			case ESCI_RXQUAL_F:
			case ESCI_RXQUAL_S:
			case ESCI_CBA:
			case ESCI_CBQ:
			case ESCI_CELL_TYPE_IND:
			case ESCI_VOCODER:
				/* FIXME implement */
				break;
		}
	}
	return 0;
}

static int _trigger_em2_do(Engineering * engineering, unsigned int * p,
		size_t cnt)
{
	if(cnt != 11)
		return 1;
	/* FIXME implement */
	return 1;
}

static int _trigger_em3_do(Engineering * engineering, char const * buf,
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
			return _do_cba(engineering, u, iter);
		case ENCI_CBQ:
			return _do_cbq(engineering, u, iter);
		case ENCI_CELL_TYPE_IND:
			return _do_cell_type_ind(engineering, u, iter);
		case ENCI_RAC:
			return _do_rac(engineering, u, iter);
		case ENCI_CELL_RESEL_OFFSET:
			return _do_cell_resel_offset(engineering, u);
		case ENCI_TEMP_OFFSET:
			return _do_temp_offset(engineering, u);
		case ENCI_RXLEV_ACC_MIN:
			return _do_rxlev_acc_min(engineering, u);
	}
	return 1;
}

static int _trigger_em4_do(Engineering * engineering, unsigned int * p,
		size_t cnt)
{
	GtkTreeIter iter;
	size_t i;
	EngineeringLocationPagingParameter elpp;
	char buf[32];

	if(cnt != 5)
		return 1;
	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(engineering->sc_store),
				&iter) == FALSE)
		gtk_list_store_append(engineering->sc_store, &iter);
	for(i = 0; i < cnt; i++)
	{
		elpp = i;
		snprintf(buf, sizeof(buf), "%u", p[i]);
		switch(elpp)
		{
			case ELPP_BS_PA_MFRMS:
			case ELPP_T3212:
			case ELPP_MCC:
			case ELPP_MNC:
				/* FIXME implement */
				break;
			case ELPP_TMSI:
				gtk_list_store_set(engineering->sc_store, &iter,
						SC_COL_TMSI, buf, -1);
				break;
		}
	}
	return 0;
}

static int _do_arfcn(Engineering * engineering, unsigned int arfcn,
		GtkTreeIter * iter)
{
	double freq;
	char buf[32];

	freq = _engineering_get_frequency(arfcn);
	snprintf(buf, sizeof(buf), "%.1lf", freq);
	gtk_list_store_set(engineering->nc_store, iter, 0, buf, -1);
	return 0;
}

static int _do_c1(Engineering * engineering, int c1, GtkTreeIter * iter)
{
	char buf[32];

	snprintf(buf, sizeof(buf), "%d", c1);
	gtk_list_store_set(engineering->nc_store, iter, NC_COL_C1, buf, -1);
	return 0;
}

static int _do_c2(Engineering * engineering, int c2, GtkTreeIter * iter)
{
	char buf[32];

	snprintf(buf, sizeof(buf), "%d", c2);
	gtk_list_store_set(engineering->nc_store, iter, NC_COL_C2, buf, -1);
	return 0;
}

static int _do_rxlev(Engineering * engineering, unsigned int rxlev,
		GtkTreeIter * iter)
{
	char buf[32];

	/* FIXME implement with a progress bar */
	snprintf(buf, sizeof(buf), "%u", (rxlev / 2) + 2);
	gtk_list_store_set(engineering->nc_store, iter, NC_COL_RXLEV, buf, -1);
	return 0;
}

static int _do_bsic(Engineering * engineering, unsigned int bsic,
		GtkTreeIter * iter)
{
	char buf[32];

	snprintf(buf, sizeof(buf), "%u", bsic);
	gtk_list_store_set(engineering->nc_store, iter, NC_COL_BSIC, buf, -1);
	return 0;
}

static int _do_cell_id(Engineering * engineering, unsigned int cell_id,
		GtkTreeIter * iter)
{
	char buf[32];

	snprintf(buf, sizeof(buf), "%u", cell_id);
	gtk_list_store_set(engineering->nc_store, iter, NC_COL_CELL_ID, buf,
			-1);
	return 0;
}

static int _do_lac(Engineering * engineering, unsigned int lac,
		GtkTreeIter * iter)
{
	char buf[32];

	snprintf(buf, sizeof(buf), "%u", lac);
	gtk_list_store_set(engineering->nc_store, iter, NC_COL_LAC, buf, -1);
	return 0;
}

static int _do_frame_offset(Engineering * engineering,
		unsigned int frame_offset, GtkTreeIter * iter)
{
	char buf[32];

	snprintf(buf, sizeof(buf), "%u", frame_offset);
	gtk_list_store_set(engineering->nc_store, iter, NC_COL_FRAME_OFFSET,
			buf, -1);
	return 0;
}

static int _do_time_alignment(Engineering * engineering,
		unsigned int time_alignment)
{
	return 0;
}

static int _do_cba(Engineering * engineering, unsigned int cba,
		GtkTreeIter * iter)
{
	char const * buf = cba ? "Yes" : "No";

	gtk_list_store_set(engineering->nc_store, iter, NC_COL_CBA, buf, -1);
	return 0;
}

static int _do_cbq(Engineering * engineering, unsigned int cbq,
		GtkTreeIter * iter)
{
	char const * buf = cbq ? "Yes" : "No";

	gtk_list_store_set(engineering->nc_store, iter, NC_COL_CBQ, buf, -1);
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
	gtk_list_store_set(engineering->nc_store, iter, NC_COL_CELL_TYPE_IND,
			type, -1);
	return 0;
}

static int _do_rac(Engineering * engineering, unsigned int rac,
		GtkTreeIter * iter)
{
	char buf[32];

	snprintf(buf, sizeof(buf), "%u", rac);
	gtk_list_store_set(engineering->nc_store, iter, NC_COL_RAC, buf, -1);
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
