/* init.c */



#include <System.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>


/* Init */
static int _init_error(char * message, int ret);
static int _init_init(void);
static int _init(void)
{
	Event * event;
	AppServer * appserver;

	if(_init_init() != 0)
		return 1;
	if((event = event_new()) == NULL)
		return _init_error("Event", 1);
	if((appserver = appserver_new_event("Session", ASO_LOCAL, event))
			== NULL)
	{
		_init_error("AppServer", 1);
		event_delete(event);
		return 1;
	}
	for(; event_loop(event) == 0;);
	appserver_delete(appserver);
	event_delete(event);
	return 0;
}

static int _init_error(char * message, int ret)
	/* FIXME should use other means to report errors */
{
	fprintf(stderr, "%s", "Init: ");
	perror(message);
	return ret;
}

static void _init_sighandler(int signum);
static int _init_init(void)
{
	struct sigaction sa;

	sa.sa_handler = _init_sighandler;
	sigfillset(&sa.sa_mask);
	if(sigaction(SIGCHLD, &sa, NULL) == -1
			|| sigaction(SIGHUP, &sa, NULL) == -1)
		return _init_error("sigaction", 1);
	return 0;
}

static void _init_sighandler(int signum)
{
	switch(signum)
	{
		case SIGCHLD:
			if(waitpid(-1, NULL, WNOHANG) == -1)
				_init_error("waitpid", 0);
			break;
		case SIGHUP:
			/* FIXME reload configuration? */
			break;
	}
}


/* usage */
static int _init_usage(void)
{
	fprintf(stderr, "%s", "Usage: Init\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	if(argc != 1)
		return _init_usage();
	return _init() == 0 ? 0 : 2;
}
