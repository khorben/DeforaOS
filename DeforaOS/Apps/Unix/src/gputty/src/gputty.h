/* gputty.h */



#ifndef _GPUTTY_H
# define _GPUTTY_H


/* GPuTTY */
/* types */
typedef struct _GPuTTY {
	/* widgets */
	GtkWidget * window;
	GtkWidget * vbox;
	/* hostname */
	GtkWidget * hn_frame;
	GtkWidget * hn_vbox;
	GtkWidget * hn_hbox;
	GtkWidget * hn_vbox1;
	GtkWidget * hn_lhostname;
	GtkWidget * hn_ehostname;
	GtkWidget * hn_vbox2;
	GtkWidget * hn_lport;
	GtkAdjustment * hn_sport_adj;
	GtkWidget * hn_sport;
	GtkWidget * hn_vbox3;
	GtkWidget * hn_lusername;
	GtkWidget * hn_eusername;
	/* sessions */
	GtkWidget * sn_frame;
	GtkWidget * sn_vbox1;
	GtkWidget * sn_lsessions;
	GtkWidget * sn_hbox;
	GtkWidget * sn_vbox2;
	GtkWidget * sn_esessions;
	GtkWidget * sn_clsessions;
	GtkWidget * sn_vbox3;
	GtkWidget * sn_load;
	GtkWidget * sn_save;
	GtkWidget * sn_delete;
	/* actions */
	GtkWidget * ac_hbox;
	GtkWidget * ac_about;
	GtkWidget * ac_quit;
	GtkWidget * ac_connect;
	/* about */
	GtkWidget * ab_window;
	GtkWidget * ab_close;
} GPuTTY;


/* functions */
GPuTTY * gputty_new(void);
void gputty_delete(GPuTTY * gputty);

/* callbacks */
void gputty_about(GtkWidget * widget, gpointer data);
void gputty_about_close(GtkWidget * widget, gpointer data);
void gputty_about_closex(GtkWidget * widget, GdkEvent * event, gpointer data);
void gputty_connect(GtkWidget * widget, gpointer data);
void gputty_quit(GtkWidget * widget, gpointer data);
void gputty_quitx(GtkWidget * widget, GdkEvent * event, gpointer data);

#endif /* !_GPUTTY_H */
