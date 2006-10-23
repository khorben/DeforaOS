/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#ifndef MAILER_COMMON_H
# define MAILER_COMMON_H

# include <gtk/gtk.h>


/* types */
struct _menu
{
	char * name;
	GtkSignalFunc callback;
	char * stock;
};

struct _menubar
{
	char * name;
	struct _menu * menu;
};


/* functions */
GtkWidget * common_new_menubar(struct _menubar * mb, gpointer data);

#endif /* !MAILER_COMMON_H */
