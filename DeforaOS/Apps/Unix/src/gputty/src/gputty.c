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
			G_CALLBACK(gtk_main_quit), NULL);
	gputty->vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(gputty->window), gputty->vbox);
	/* hostname */
	gputty->hn_frame = gtk_frame_new("Specify your connection");
	gtk_container_set_border_width(GTK_CONTAINER(gputty->hn_frame), 3);
	gtk_box_pack_start(GTK_BOX(gputty->vbox), gputty->hn_frame, TRUE, FALSE, 0);
	gputty->hn_vbox = gtk_vbox_new(FALSE, 0);
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
	gtk_container_set_border_width(GTK_CONTAINER(gputty->sn_frame), 3);
	gtk_box_pack_start(GTK_BOX(gputty->vbox), gputty->sn_frame, TRUE, TRUE, 0);
	gputty->sn_hbox = gtk_hbox_new(TRUE, 0);
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
	gtk_widget_show_all(gputty->window);
	return gputty;
}


void gputty_delete(GPuTTY * gputty)
{
	free(gputty);
}


/* main */
int main(int argc, char * argv[])
{
	GPuTTY * gputty;

	gtk_init(&argc, &argv);
	if((gputty = gputty_new()) == NULL)
		return 1;
	gtk_main();
	gputty_delete(gputty);
	return 0;
}
