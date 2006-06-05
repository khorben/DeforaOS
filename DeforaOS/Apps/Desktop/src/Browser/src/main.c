/* main.c */



#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include "browser.h"


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: browser [directory]\n");
	return 1;
}


/* main */
static void _main_sigchld(int signum);
int main(int argc, char * argv[])
{
	int o;
	Browser * browser;
	struct sigaction sa;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind < argc-1)
		return _usage();
	browser = browser_new(argv[optind]);
	sa.sa_handler = _main_sigchld;
	sigfillset(&sa.sa_mask);
	if(sigaction(SIGCHLD, &sa, NULL) == -1)
		browser_error(browser, "signal handling error", 0);
	gtk_main();
	if(browser != NULL)
		browser_delete(browser);
	return 0;
}

static void _main_sigchld(int signum)
{
	wait(NULL);
}
