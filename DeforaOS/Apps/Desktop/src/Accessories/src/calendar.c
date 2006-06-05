/* calendar.c */



#include <gtk/gtk.h>


/* Calendar */
static void _calendar_on_exitx(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static int _calendar(void)
{
	GtkWidget * window;
	GtkWidget * calendar;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Calendar");
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
				_calendar_on_exitx), NULL);
	calendar = gtk_calendar_new();
	gtk_container_add(GTK_CONTAINER(window), calendar);
	gtk_widget_show_all(window);
	gtk_main();
	return 0;
}

static void _calendar_on_exitx(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_widget_hide(widget);
	gtk_main_quit();
}


/* main */
int main(int argc, char * argv[])
{
	gtk_init(&argc, &argv);
	return _calendar() == 0 ? 0 : 2;
}
