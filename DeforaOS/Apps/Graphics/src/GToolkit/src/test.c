#include <GToolkit.h>


/* main */
int main(void)
{
	GWindow * window;

	if(g_init() != 0)
		return 2;
	window = gwindow_new();
	gwindow_show(window);
	g_main();
	g_quit();
	return 0;
}
