/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Keyboard */
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
#ifdef DEBUG
# include <stdio.h>
#endif
#define XK_LATIN1
#define XK_MISCELLANY
#include <X11/keysymdef.h>
#include <X11/extensions/XTest.h>
#include <gdk/gdkx.h>
#include "common.h"
#include "layout.h"


/* KeyboardLayout */
/* private */
/* types */
typedef struct _KeyboardKeyRow KeyboardKeyRow;

struct _KeyboardLayout
{
	KeyboardKeyRow * rows;
	size_t rows_cnt;

	/* widgets */
	GtkWidget * widget;
};

struct _KeyboardKeyRow
{
	KeyboardKey ** keys;
	size_t keys_cnt;
	unsigned int width;
};


/* prototypes */
/* callbacks */
static void _on_key_clicked(GtkWidget * widget, gpointer data);


/* public */
/* functions */
KeyboardLayout * keyboard_layout_new(void)
{
	KeyboardLayout * layout;

	if((layout = malloc(sizeof(*layout))) == NULL)
		return NULL;
	layout->rows = NULL;
	layout->rows_cnt = 0;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() gtk_table_new(%u, %u)\n", __func__, 1, 1);
#endif
	layout->widget = gtk_table_new(1, 1, TRUE);
	return layout;
}


/* keyboard_layout_get_widget */
GtkWidget * keyboard_layout_get_widget(KeyboardLayout * layout)
{
	return layout->widget;
}


/* useful */
/* keyboard_layout_add */
KeyboardKey * keyboard_layout_add(KeyboardLayout * layout, unsigned int row,
		unsigned int width, unsigned int keysym, char const * label)
{
	KeyboardKey * ret = NULL;
	KeyboardKeyRow * p = &layout->rows[row];
	KeyboardKey ** q;
	GtkAttachOptions options = GTK_EXPAND | GTK_SHRINK | GTK_FILL;
	GtkWidget * widget;

	if(row >= layout->rows_cnt)
	{
		if((p = realloc(layout->rows, sizeof(*p) * (row + 1))) == NULL)
			return NULL;
		layout->rows = p;
		for(; layout->rows_cnt <= row; layout->rows_cnt++)
		{
			layout->rows[layout->rows_cnt].keys = NULL;
			layout->rows[layout->rows_cnt].keys_cnt = 0;
			layout->rows[layout->rows_cnt].width = 0;
		}
		p = &layout->rows[row];
	}
	if((q = realloc(p->keys, sizeof(*q) * (p->keys_cnt + 1))) == NULL)
		return NULL;
	p->keys = q;
	q = &p->keys[p->keys_cnt];
	if(keysym != 0 && label != NULL)
	{
		if((ret = keyboard_key_new(keysym, label)) == NULL)
			return NULL;
		widget = keyboard_key_get_widget(ret);
		g_object_set_data(G_OBJECT(widget), "key", ret);
		g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
					_on_key_clicked), layout);
		if(width == 0)
			width = 1;
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() gtk_table_resize(%u, %u)\n",
				__func__, (unsigned)layout->rows_cnt,
				(unsigned)p->width + width);
		fprintf(stderr, "DEBUG: %s() %s(%u, %u, %u, %u)\n", __func__,
				"gtk_table_attach", p->width, p->width + width,
				row, row + 1);
#endif
		gtk_table_resize(GTK_TABLE(layout->widget),
				layout->rows_cnt, p->width + width);
		gtk_table_attach(GTK_TABLE(layout->widget), widget, p->width,
				p->width + width, row, row + 1, options,
				options, 2, 2);
		p->keys[p->keys_cnt++] = ret;
	}
	p->width+=width;
	return ret;
}


/* keyboard_layout_add_widget */
void keyboard_layout_add_widget(KeyboardLayout * layout, unsigned int row,
		unsigned int column, unsigned int width, GtkWidget * widget)
{
	GtkAttachOptions options = GTK_EXPAND | GTK_SHRINK | GTK_FILL;

	gtk_table_attach(GTK_TABLE(layout->widget), widget, column, width, row,
			row + 1, options, options, 2, 2);
}


/* keyboard_layout_apply_modifier */
void keyboard_layout_apply_modifier(KeyboardLayout * layout,
		unsigned int modifier)
{
	size_t i;
	size_t j;

	for(i = 0; i < layout->rows_cnt; i++)
		for(j = 0; j < layout->rows[i].keys_cnt; j++)
			keyboard_key_apply_modifier(layout->rows[i].keys[j],
					modifier);
}


/* private */
/* functions */
/* on_key_clicked */
static void _on_key_clicked(GtkWidget * widget, gpointer data)
{
	KeyboardLayout * layout = data;
	KeyboardKey * key;
	Display * display;
	KeySym keysym;
	KeyCode keycode;
	gboolean active;

	key = g_object_get_data(G_OBJECT(widget), "key");
	keysym = keyboard_key_get_keysym(key);
	display = gdk_x11_get_default_xdisplay();
	if((keycode = XKeysymToKeycode(display, keysym)) == NoSymbol)
		return;
	XTestGrabControl(display, True);
	if(keysym_is_modifier(keysym) != 0)
	{
		active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
					widget));
		if(keysym != XK_Num_Lock) /* XXX ugly workaround */
			XTestFakeKeyEvent(display, keycode, active ? True
					: False, 0);
		else
		{
			XTestFakeKeyEvent(display, keycode, True, 0);
			XTestFakeKeyEvent(display, keycode, False, 0);
		}
		keyboard_layout_apply_modifier(layout, active ? keysym : 0);
	}
	else
	{
		XTestFakeKeyEvent(display, keycode, True, 0);
		XTestFakeKeyEvent(display, keycode, False, 0);
	}
	XTestGrabControl(display, False);
}
