/* time.c */



#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
extern int optind;
#include <stdio.h>
#include <errno.h>


/* time */
static int _time_error(char * error, int ret);
static int _time(int argc, char * argv[])
{
	pid_t pid;
	int status;
	struct tms tmsbuf;
	clock_t cbefore, cafter;

	if((cbefore = times(NULL)) == -1)
		return _time_error("times", 2);
	if((pid = fork()) == -1)
		return _time_error("fork", 2);
	if(pid == 0)
	{
		execvp(argv[0], argv);
		if(errno == ENOENT)
			return _time_error(argv[0], 127);
		return _time_error(argv[0], 126);
	}
	for(;;)
	{
		if(waitpid(pid, &status, 0) == -1)
			return _time_error("waitpid", 2);
		if(WIFEXITED(status))
			break;
	}
	if((cafter = times(&tmsbuf)) == -1)
		return _time_error("times", 2);
	/* FIXME */
	fprintf(stderr, "real %ld\nuser %ld\nsys %ld\n",
			cafter - cbefore,
			tmsbuf.tms_utime + tmsbuf.tms_cutime,
			tmsbuf.tms_stime + tmsbuf.tms_cstime);
	return 0;
}

static int _time_error(char * message, int ret)
{
	fprintf(stderr, "%s", "time: ");
	perror(message);
	return ret;
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
		if(o == '?')
			return _usage();
	}
	if(optind == argc)
		return _usage();
	return _time(argc - optind, &argv[optind]);
}
