/* inetd.c */



#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include "parser.h"
#include "inetd.h"


/* variables */
InetdState * inetd_state;


/* inetd_error */
int inetd_error(char const * message, int ret)
{
	fprintf(stderr, "%s", "inetd: ");
	perror(message);
	return ret;
}


/* inetd */
static int _inetd_setup(InetdState * state, char * filename);
static int _inetd_do(InetdState * state);
static int _inetd(int debug, int queue, char * filename)
{
	InetdState state;
	int ret;

	inetd_state = &state;
	state.debug = debug;
	state.queue = queue;
	state.filename = filename;
	if(_inetd_setup(&state, filename))
		return 2;
	ret = _inetd_do(&state);
	config_delete(state.config);
	return ret;
}

static void _inetd_sighandler(int signum);
static int _inetd_setup(InetdState * state, char * config)
{
	if(signal(SIGCHLD, _inetd_sighandler) == SIG_ERR
			|| signal(SIGHUP, _inetd_sighandler) == SIG_ERR)
		return inetd_error("signal", 1);
	if((state->config = parser(config)) == NULL)
		return 1;
	return 0;
}

static void _inetd_sigchld(void);
static void _inetd_sighup(void);
static void _inetd_sighandler(int signum)
{
	if(signal(signum, _inetd_sighandler) == SIG_ERR)
		inetd_error("signal", 0);
	switch(signum)
	{
		case SIGCHLD:
			_inetd_sigchld();
			break;
		case SIGHUP:
			_inetd_sighup();
			break;
	}
}

static void _inetd_sigchld(void)
{
	pid_t pid;
	int status;

	if((pid = waitpid(-1, &status, WNOHANG)) == -1)
	{
		inetd_error("waitpid", 0);
		return;
	}
	if(inetd_state->debug)
		fprintf(stderr, "%s%d%s%s%d%s", "Child ", pid, WIFEXITED(status)
				? " exited" : " was terminated",
				" with error code ", WEXITSTATUS(status), "\n");
}

static void _inetd_sighup(void)
{
	Config * config;

	if((config = parser(inetd_state->filename)) == NULL)
	{
		if(inetd_state->debug)
			fprintf(stderr, "%s%s%s", "inetd: ",
					inetd_state->filename,
					"Ignoring reconfiguration request\n");
		return;
	}
	config_delete(inetd_state->config);
	inetd_state->config = config;
	/* FIXME */
	/* close all active fd to stop select and use the stack reliably, but
	 * it will certainly result in an error => handle it... */
}

static int _inetd_do(InetdState * state)
{
	fd_set rfds;
	fd_set rfdstmp;
	unsigned int i;
	int hifd = -1;

	FD_ZERO(&rfds);
	for(i = 0; i < state->config->services_nb; i++)
	{
		if(service_listen(state->config->services[i]))
			continue;
		FD_SET(state->config->services[i]->fd, &rfds);
		hifd = hifd >= state->config->services[i]->fd ? hifd
			: state->config->services[i]->fd;
	}
	for(rfdstmp = rfds;; rfds = rfdstmp)
	{
		if(state->debug)
			fprintf(stderr, "%s%d%s", "select(", hifd+1, ")\n");
		if(select(hifd+1, &rfds, NULL, NULL, NULL) == -1)
		{
			if(errno != EINTR)
				return inetd_error("select", 2);
			continue;
		}
		for(i = 0; i < state->config->services_nb; i++)
			if(FD_ISSET(state->config->services[i]->fd, &rfds))
				service_exec(state->config->services[i]);
	}
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: inetd [-d][-q len] [config file]\n\
  -d	Debugging mode\n\
  -q	Queue length\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	int debug = 0;
	int queue = 128;
	char * filename = "/etc/inetd.conf";
	char * p;

	while((o = getopt(argc, argv, "dq")) != -1)
	{
		if(o == 'd')
			debug = 1;
		else if(o == 'q')
		{
			queue = strtol(optarg, &p, 10);
			if(*optarg == '\0' || *p != '\0')
				return _usage();
		}
		else
			return _usage();
	}
	if(argc - optind == 1)
		filename = argv[optind];
	else if(argc != optind)
		return _usage();
	return _inetd(debug, queue, filename) ? 2 : 0;
}
