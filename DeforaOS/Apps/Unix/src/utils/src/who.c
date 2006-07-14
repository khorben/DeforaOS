/* who.c */



#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <utmpx.h>


/* types */
typedef int Prefs;
#define PREFS_m 0x1
#define PREFS_T 0x2


/* who */
static int _who_error(char const * message, int ret);
static char * _who_tty(void);
static int _who(Prefs * prefs)
{
	struct utmpx * u;
	struct tm * tm;
	char buf[13];
	char * tty = NULL;
	time_t t;

	memset(&tm, 0, sizeof(tm));
	if(*prefs & PREFS_m && (tty = _who_tty()) == NULL)
		return 1;
	for(; (u = getutxent()) != NULL;)
	{
		if(u->ut_type != USER_PROCESS)
			continue;
		if(tty != NULL && strcmp(tty, u->ut_line) != 0)
			continue;
		printf("%-8s", u->ut_user);
		if(*prefs & PREFS_T)
			printf(" %c", '?');
		printf(" %-8s", u->ut_line);
		t = u->ut_tv.tv_sec;
		if((tm = localtime(&t)) == NULL || strftime(buf,
					sizeof(buf), "%b %e %H:%M", tm) == 0)
			strcpy(buf, "n/a");
		printf(" %s\n", buf);
	}
	return 0;
}

static int _who_error(char const * message, int ret)
{
	fprintf(stderr, "%s", "ttyname: ");
	perror(message);
	return ret;
}

static char * _who_tty(void)
{
	char * tty;

	if((tty = ttyname(0)) == NULL)
	{
		_who_error("ttyname", 1);
		return NULL;
	}
	if(strncmp(tty, "/dev/", 5) != 0)
		return tty;
	return tty + 5;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: who\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;

	memset(&prefs, 0, sizeof(prefs));
	while((o = getopt(argc, argv, "mTu")) != -1)
		switch(o)
		{
			case 'm':
				prefs |= PREFS_m;
				break;
			case 'T':
				prefs |= PREFS_T;
				break;
			default:
				return _usage();
		}
	return _who(&prefs) == 0 ? 0 : 2;
}
