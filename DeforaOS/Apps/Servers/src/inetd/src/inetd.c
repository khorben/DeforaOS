/* inetd.c */



#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "config.h"
#include "parser.h"


/* types */
typedef struct _InetdState {
	int debug;
	int queue;
	Config * config;
} InetdState;


/* inetd_error */
int inetd_error(char const * message, int ret)
{
	fprintf(stderr, "%s", "inetd: ");
	perror(message);
	return ret;
}


/* inetd */
static int _inetd_setup(InetdState * state, char * config);
static int _inetd_do(InetdState * state);
static int _inetd(int debug, int queue, char * config)
{
	InetdState state;

	state.debug = debug;
	state.queue = queue;
	if(_inetd_setup(&state, config))
		return 2;
	return _inetd_do(&state);
}

static void _inetd_sighandler(int signum);
static int _inetd_setup(InetdState * state, char * config)
{
	if(signal(SIGCHLD, _inetd_sighandler) == SIG_ERR)
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
	/* FIXME only in debugging mode */
	fprintf(stderr, "%s%d%s%s%d%s", "Child ", pid,
			WIFEXITED(status) ? " exited" : " was terminated",
			" with error code ", WEXITSTATUS(status), "\n");
}

static void _inetd_sighup(void)
{
	/* FIXME */
}

/* static void _inetd_accept(int fd, struct sockaddr_in addr, int addrlen); */
static int _inetd_do(InetdState * state)
{
/*	struct sockaddr_in sa_conn;
	int sa_size = sizeof(struct sockaddr_in);
	int conn;

	while((conn = accept(fd, &sa_conn, &sa_size)) != 0)
		_inetd_accept(conn, sa_conn, sa_size); */
	config_delete(state->config);
	return 0;
}

/* static void _inetd_log(struct sockaddr_in * addr, int addrlen); */
/* static void _inetd_accept(int fd, struct sockaddr_in addr, int addrlen)
{
	pid_t pid;

	if((pid = fork()) == -1)
	{
		inetd_error("fork", 0);
		return;
	}
	if(pid > 0)
	{
		_inetd_log(&addr, addrlen); //FIXME only in debugging mode
		close(fd);
		return;
	}
} */

/* static void _inetd_log(struct sockaddr_in * addr, int addrlen)
{
	uint8_t * ip = (uint8_t*)&(addr->sin_addr.s_addr);

	fprintf(stderr, "%s%d.%d.%d.%d:%d\n", "Connection from ",
			ip[0], ip[1], ip[2], ip[3], ntohs(addr->sin_port));
} */


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
	char * config = "/etc/inetd.conf";
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
		config = argv[optind];
	else if(argc != optind)
		return _usage();
	return _inetd(debug, queue, config) ? 2 : 0;
}
