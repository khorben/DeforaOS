/* time.c */



#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>


/* time */
static int _time_error(char * error, int ret);
static int _time_exec(char * argv[]);
static int _time_print(long real, long user, long sys);
static int _time(char * argv[])
{
	pid_t pid;
	int status;
	struct tms tmsbuf;
	clock_t cbefore, cafter;

	if((cbefore = times(NULL)) == (clock_t)-1)
		return _time_error("times", 2);
	if((pid = fork()) == -1)
		return _time_error("fork", 2);
	if(pid == 0)
		return _time_exec(argv);
	for(;;)
	{
		if(waitpid(pid, &status, 0) == -1)
			return _time_error("waitpid", 2);
		if(WIFEXITED(status))
			break;
	}
	if((cafter = times(&tmsbuf)) == (clock_t)-1)
		return _time_error("times", 2);
	return _time_print(cafter - cbefore,
			tmsbuf.tms_utime + tmsbuf.tms_cutime,
			tmsbuf.tms_stime + tmsbuf.tms_cstime);
}

static int _time_error(char * message, int ret)
{
	fprintf(stderr, "%s", "time: ");
	perror(message);
	return ret;
}

static int _time_exec(char * argv[])
{
	execvp(argv[0], argv);
	if(errno == ENOENT)
		return _time_error(argv[0], 127);
	return _time_error(argv[0], 126);
}

static int _time_print(long real, long user, long sys)
{
	char * args[3] = { "real", "user", "sys" };
	long * argl[3] = { &real, &user, &sys };
	int i;
	long l;
	long r;

	if((r = sysconf(_SC_CLK_TCK)) == -1)
	{
		_time_error("sysconf", 0);
		r = 100;
	}
	for(i = 0; i < 3; i++)
	{
		l = *argl[i] / r;
		if(l * r > *argl[i])
			l--;
		/* FIXME */
		fprintf(stderr, "%s %ld.%02lds\n", args[i], l, *argl[i] % r);
	}
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: time [-p] utility [argument...]\n\
  -p    force the POSIX locale\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;

	while((o = getopt(argc, argv, "p")) != -1)
	{
		if(o == 'p')
			continue;
		return _usage();
	}
	if(optind == argc)
		return _usage();
	return _time(&argv[optind]);
}
