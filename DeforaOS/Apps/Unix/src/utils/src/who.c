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
static int _who(Prefs * prefs)
{
	struct utmpx * u;
	struct tm * tm;
	char buf[13];
	char * tty = NULL;

	memset(&tm, 0, sizeof(tm));
	if(*prefs & PREFS_m)
	{
		if((tty = ttyname(0)) == NULL)
			return _who_error("ttyname", 1);
		if(strncmp(tty, "/dev/", 5) != 0)
			tty = NULL;
		else
			tty+=5;
	}
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
		if((tm = localtime(&u->ut_tv.tv_sec)) == NULL || strftime(buf,
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
