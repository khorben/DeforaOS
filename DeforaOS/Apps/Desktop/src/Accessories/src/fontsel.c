/* fontsel.c */



#include <gtk/gtk.h>


/* Font */
static void _fontsel_on_exitx(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static int _fontsel(void)
{
	GtkWidget * window;
	GtkWidget * fontsel;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_window_set_title(GTK_WINDOW(window), "Font browser");
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
				_fontsel_on_exitx), NULL);
	fontsel = gtk_font_selection_new();
	gtk_container_add(GTK_CONTAINER(window), fontsel);
	gtk_widget_show_all(window);
	gtk_main();
	return 0;
}

static void _fontsel_on_exitx(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_widget_hide(widget);
	gtk_main_quit();
}


/* main */
int main(int argc, char * argv[])
{
	gtk_init(&argc, &argv);
	return _fontsel() == 0 ? 0 : 2;
}
