/* gputty.c */



#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
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
	gtk_container_set_border_width(GTK_CONTAINER(gputty->window), 2);
	g_signal_connect(G_OBJECT(gputty->window), "delete_event",
			G_CALLBACK(gputty_quitx), gputty);
	gputty->vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(gputty->window), gputty->vbox);
	/* hostname */
	gputty->hn_frame = gtk_frame_new("Specify your connection");
	gtk_container_set_border_width(GTK_CONTAINER(gputty->hn_frame), 2);
	gtk_box_pack_start(GTK_BOX(gputty->vbox), gputty->hn_frame, FALSE, FALSE, 0);
	gputty->hn_vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(gputty->hn_frame), gputty->hn_vbox);
	gputty->hn_hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(gputty->hn_hbox), 2);
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
	gputty->sn_vbox1 = gtk_vbox_new(FALSE, 0);
	gputty->sn_lsessions = gtk_label_new("Saved sessions");
	gtk_box_pack_start(GTK_BOX(gputty->sn_vbox1), gputty->sn_lsessions, FALSE, FALSE, 0);
	gputty->sn_hbox = gtk_hbox_new(FALSE, 0);
	/* sessions: list */
	gputty->sn_vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(gputty->sn_vbox2), 2);
	gputty->sn_esessions = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(gputty->sn_vbox2), gputty->sn_esessions, FALSE, FALSE, 0);
	gputty->sn_clsessions = gtk_clist_new(1);
	gtk_box_pack_start(GTK_BOX(gputty->sn_vbox2), gputty->sn_clsessions, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gputty->sn_hbox), gputty->sn_vbox2, TRUE, TRUE, 0);
	/* sessions: buttons */
	gputty->sn_vbox3 = gtk_vbox_new(FALSE, 0);
	gputty->sn_load = gtk_button_new_with_label("Load");
	gtk_container_set_border_width(GTK_CONTAINER(gputty->sn_load), 2);
	gtk_box_pack_start(GTK_BOX(gputty->sn_vbox3), gputty->sn_load, FALSE, FALSE, 0);
	gputty->sn_save = gtk_button_new_with_label("Save");
	gtk_container_set_border_width(GTK_CONTAINER(gputty->sn_save), 2);
	gtk_box_pack_start(GTK_BOX(gputty->sn_vbox3), gputty->sn_save, FALSE, FALSE, 0);
	gputty->sn_delete = gtk_button_new_with_label("Delete");
	gtk_container_set_border_width(GTK_CONTAINER(gputty->sn_delete), 2);
	gtk_box_pack_start(GTK_BOX(gputty->sn_vbox3), gputty->sn_delete, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gputty->sn_hbox), gputty->sn_vbox3, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gputty->sn_vbox1), gputty->sn_hbox, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(gputty->sn_frame), gputty->sn_vbox1);
	gtk_box_pack_start(GTK_BOX(gputty->vbox), gputty->sn_frame, TRUE, TRUE, 0);
	/* actions */
	gputty->ac_hbox = gtk_hbox_new(FALSE, 0);
	gputty->ac_about = gtk_button_new_with_label("About");
	gtk_box_pack_start(GTK_BOX(gputty->ac_hbox), gputty->ac_about, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(gputty->ac_about), "clicked",
			G_CALLBACK(gputty_about), gputty);
	gputty->ac_connect = gtk_button_new_with_label("Connect");
	g_signal_connect(G_OBJECT(gputty->ac_connect), "clicked",
			G_CALLBACK(gputty_connect), gputty);
	gtk_box_pack_end(GTK_BOX(gputty->ac_hbox), gputty->ac_connect, FALSE, FALSE, 0);
	gputty->ac_quit = gtk_button_new_with_label("Quit");
	gtk_box_pack_end(GTK_BOX(gputty->ac_hbox), gputty->ac_quit, FALSE, FALSE, 2);
	g_signal_connect(G_OBJECT(gputty->ac_quit), "clicked",
			G_CALLBACK(gputty_quit), gputty);
	gtk_box_pack_start(GTK_BOX(gputty->vbox), gputty->ac_hbox, FALSE, FALSE, 0);
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

void gputty_connect(GtkWidget * widget, gpointer data)
{
	GPuTTY * g = data;
	pid_t pid;
	char * xterm;
	char * hostname;
	char port[6];
	char * useropt = NULL;
	char * username = NULL;

	xterm = "xterm";
	hostname = gtk_entry_get_text(GTK_ENTRY(g->hn_ehostname));
	if(snprintf(port, 6, "%d", gtk_spin_button_get_value_as_int(g->hn_sport)) >= 6)
		port[5] = '\0';
	if(hostname[0] == '\0')
		return; /* FIXME warn user */
	if((pid = fork()) == -1)
	{
		warn("fork");
		return;
	}
	else if(pid == 0)
	{
		username = gtk_entry_get_text(GTK_ENTRY(g->hn_eusername));
		if(username[0] != '\0')
			useropt = "-l";
		execlp(xterm, xterm, "-e",
				"ssh", hostname,
				"-p", port,
				useropt, username,
				NULL);
		err(2, xterm);
	}
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
