/* time.c */



#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
extern int optind;
#include <stdio.h>
#include <errno.h>


/* time */
static int _time(int argc, char * argv[])
{
	pid_t pid;
	int status;
	struct tms tmsbuf;
	clock_t cbefore, cafter;

	if((cbefore = times(NULL)) == -1)
	{
		fprintf(stderr, "%s", "time: ");
		perror("times");
		return 2;
	}
	if((pid = fork()) == -1)
	{
		fprintf(stderr, "%s", "time: ");
		perror("fork");
		return 2;
	}
	if(pid == 0)
	{
		execvp(argv[0], argv);
		fprintf(stderr, "%s", "time: ");
		perror(argv[0]);
		if(errno == ENOENT)
			return 127;
		return 126;
	}
	for(;;)
	{
		if(waitpid(pid, &status, 0) == -1)
		{
			fprintf(stderr, "%s", "time: ");
			perror("waitpid");
			return 2;
		}
		if(WIFEXITED(status))
			break;
	}
	if((cafter = times(&tmsbuf)) == -1)
	{
		fprintf(stderr, "%s", "time: ");
		perror("times");
		return 2;
	}
	/* FIXME */
	fprintf(stderr, "real %ld\nuser %ld\nsys %ld\n",
			cafter - cbefore,
			tmsbuf.tms_utime + tmsbuf.tms_cutime,
			tmsbuf.tms_stime + tmsbuf.tms_cstime);
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
		if(o == '?')
			return _usage();
	}
	if(optind == argc)
		return _usage();
	return _time(argc - optind, &argv[optind]);
}
