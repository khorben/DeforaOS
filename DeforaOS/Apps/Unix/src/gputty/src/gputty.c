/* gputty.c */



#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "gputty.h"


/* GPuTTY */
GPuTTY * gputty_new(void)
{
	GPuTTY * gputty;

	if((gputty = malloc(sizeof(GPuTTY))) == NULL)
	{
		perror("malloc");
		return NULL;
	}

	/* widgets */
	gputty->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(gputty->window), "GPuTTY configuration");
	g_signal_connect(G_OBJECT(gputty->window), "delete_event",
			G_CALLBACK(gputty_quitx), gputty);
	gputty->vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(gputty->window), gputty->vbox);
	/* hostname */
	gputty->hn_frame = gtk_frame_new("Specify your connection");
	gtk_container_set_border_width(GTK_CONTAINER(gputty->hn_frame), 2);
	gtk_box_pack_start(GTK_BOX(gputty->vbox), gputty->hn_frame, TRUE, FALSE, 0);
	gputty->hn_vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(gputty->hn_vbox), 2);
	gtk_container_add(GTK_CONTAINER(gputty->hn_frame), gputty->hn_vbox);
	gputty->hn_hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gputty->hn_vbox), gputty->hn_hbox, TRUE, TRUE, 0);
	/* hostname: hostname */
	gputty->hn_vbox1 = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gputty->hn_hbox), gputty->hn_vbox1, TRUE, TRUE, 0);
	gputty->hn_lhostname = gtk_label_new("Host name");
	gtk_box_pack_start(GTK_BOX(gputty->hn_vbox1), gputty->hn_lhostname, TRUE, TRUE, 0);
	gputty->hn_ehostname = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(gputty->hn_vbox1), gputty->hn_ehostname, TRUE, TRUE, 0);
	/* hostname: port */
	gputty->hn_vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gputty->hn_hbox), gputty->hn_vbox2, TRUE, TRUE, 0);
	gputty->hn_lport = gtk_label_new("Port");
	gtk_box_pack_start(GTK_BOX(gputty->hn_vbox2), gputty->hn_lport, TRUE, TRUE, 0);
	gputty->hn_sport_adj = (GtkAdjustment *)gtk_adjustment_new(22, 0, 65535, 1, 4, 4);
	gputty->hn_sport = gtk_spin_button_new(gputty->hn_sport_adj, 1, 0);
	gtk_box_pack_start(GTK_BOX(gputty->hn_vbox2), gputty->hn_sport, TRUE, TRUE, 0);
	/* hostname: username */
	gputty->hn_vbox3 = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gputty->hn_hbox), gputty->hn_vbox3, TRUE, TRUE, 0);
	gputty->hn_lusername = gtk_label_new("User name");
	gtk_box_pack_start(GTK_BOX(gputty->hn_vbox3), gputty->hn_lusername, TRUE, TRUE, 0);
	gputty->hn_eusername = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(gputty->hn_vbox3), gputty->hn_eusername, TRUE, TRUE, 0);
	/* sessions */
	gputty->sn_frame = gtk_frame_new("Manage sessions");
	gtk_container_set_border_width(GTK_CONTAINER(gputty->sn_frame), 2);
	gtk_box_pack_start(GTK_BOX(gputty->vbox), gputty->sn_frame, TRUE, TRUE, 0);
	gputty->sn_hbox = gtk_hbox_new(TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(gputty->sn_hbox), 2);
	gtk_container_add(GTK_CONTAINER(gputty->sn_frame), gputty->sn_hbox);
	gputty->sn_vbox1 = gtk_vbox_new(TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gputty->sn_hbox), gputty->sn_vbox1, TRUE, TRUE, 0);
	gputty->sn_lsessions = gtk_label_new("Saved sessions");
	gtk_box_pack_start(GTK_BOX(gputty->sn_vbox1), gputty->sn_lsessions, TRUE, TRUE, 0);
	gputty->sn_esessions = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(gputty->sn_vbox1), gputty->sn_esessions, TRUE, TRUE, 0);
	gputty->sn_clsessions = gtk_clist_new(1);
	gtk_box_pack_start(GTK_BOX(gputty->sn_vbox1), gputty->sn_clsessions, TRUE, TRUE, 0);
	gputty->sn_vbox2 = gtk_vbox_new(TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gputty->sn_hbox), gputty->sn_vbox2, TRUE, TRUE, 0);
	gputty->sn_load = gtk_button_new_with_label("Load");
	gtk_box_pack_start(GTK_BOX(gputty->sn_vbox2), gputty->sn_load, TRUE, TRUE, 0);
	gputty->sn_save = gtk_button_new_with_label("Save");
	gtk_box_pack_start(GTK_BOX(gputty->sn_vbox2), gputty->sn_save, TRUE, TRUE, 0);
	gputty->sn_delete = gtk_button_new_with_label("Delete");
	gtk_box_pack_start(GTK_BOX(gputty->sn_vbox2), gputty->sn_delete, TRUE, TRUE, 0);
	/* actions */
	gputty->ac_hbox = gtk_hbox_new(FALSE, 0);
	gputty->ac_about = gtk_button_new_with_label("About");
	gtk_box_pack_start(GTK_BOX(gputty->ac_hbox), gputty->ac_about, TRUE, FALSE, 0);
	g_signal_connect(G_OBJECT(gputty->ac_about), "clicked",
			G_CALLBACK(gputty_about), gputty);
	gputty->ac_connect = gtk_button_new_with_label("Connect");
	gtk_box_pack_end(GTK_BOX(gputty->ac_hbox), gputty->ac_connect, TRUE, FALSE, 0);
	gputty->ac_quit = gtk_button_new_with_label("Quit");
	gtk_box_pack_end(GTK_BOX(gputty->ac_hbox), gputty->ac_quit, TRUE, FALSE, 0);
	g_signal_connect(G_OBJECT(gputty->ac_quit), "clicked",
			G_CALLBACK(gputty_quit), gputty);
	gtk_box_pack_start(GTK_BOX(gputty->vbox), gputty->ac_hbox, TRUE, FALSE, 0);
	/* about */
	gputty->ab_window = NULL;
	gtk_widget_show_all(gputty->window);
	return gputty;
}


void gputty_delete(GPuTTY * gputty)
{
	free(gputty);
}


/* callbacks */
void gputty_about(GtkWidget * widget, gpointer data)
{
	GPuTTY * g;

	g = data;
	if(g->ab_window == NULL)
	{
		g->ab_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(g->ab_window), "About GPuTTY");
		g_signal_connect(G_OBJECT(g->ab_window), "delete_event",
				G_CALLBACK(gputty_about_closex), g);
		g->ab_close = gtk_button_new_with_label("Close");
		g_signal_connect(G_OBJECT(g->ab_close), "clicked",
				G_CALLBACK(gputty_about_close), g);
		gtk_container_add(GTK_CONTAINER(g->ab_window), g->ab_close);
	}
	gtk_widget_show_all(g->ab_window);
}

void gputty_about_close(GtkWidget * widget, gpointer data)
{
	GPuTTY * g;

	g = data;
	gtk_widget_hide(g->ab_window);
}

void gputty_about_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	gputty_about_close(widget, data);
}

void gputty_quit(GtkWidget * widget, gpointer data)
{
	gputty_delete(data);
	gtk_main_quit();
}

void gputty_quitx(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	gputty_quit(widget, data);
}


/* main */
int main(int argc, char * argv[])
{
	GPuTTY * gputty;

	gtk_init(&argc, &argv);
	if((gputty = gputty_new()) == NULL)
		return 1;
	gtk_main();
	return 0;
}
