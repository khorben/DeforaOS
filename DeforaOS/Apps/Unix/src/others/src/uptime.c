/* uptime.c */



#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <utmpx.h>


/* uptime */
static int _uptime_error(char const * message, int ret);
static int _uptime(void)
{
	struct timeval tv;
	struct tm * tm;
	char time[9];
	unsigned int nusers;
	struct utmpx * u;
	double loadavg[3];

	if((gettimeofday(&tv, NULL)) != 0)
		return _uptime_error("gettimeofday", 1);
	if((tm = gmtime(&tv.tv_sec)) == NULL)
		return _uptime_error("gmtime", 1);
	if(strftime(time, sizeof(time), "%X", tm) == 0)
		return _uptime_error("strftime", 1);
	/* FIXME uptime is not portable afaik */
	for(nusers = 0; (u = getutxent()) != NULL;)
		if(u->ut_type == USER_PROCESS)
			nusers++;
	if(getloadavg(&loadavg, 3) != 3)
		return _uptime_error("getloadavg", 1);
	printf(" %s up %3d%s%.2f, %.2f, %.2f\n", time, nusers,
			" users, load average: ", loadavg[0], loadavg[1],
			loadavg[2]);
	return 0;
}

static int _uptime_error(char const * message, int ret)
{
	fprintf(stderr, "%s", "uptime: ");
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: uptime\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	if(argc != 1)
		return _usage();
	return _uptime() == 0 ? 0 : 2;
}
