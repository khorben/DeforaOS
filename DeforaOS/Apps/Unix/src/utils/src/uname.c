/* uname.c */



#include <sys/utsname.h>
#include <unistd.h>
#include <stdio.h>


/* uname */
static int _uname(int m, int n, int r, int s, int v)
{
	struct utsname buf;
	int spacing = 0;

	if(uname(&buf) == -1)
	{
		perror("uname");
		return 2;
	}
	if(s && (spacing = 1))
		printf("%s", buf.sysname);
	if(n)
		printf("%s%s", spacing++ ? " " : "", buf.nodename);
	if(r)
		printf("%s%s", spacing++ ? " " : "", buf.release);
	if(v)
		printf("%s%s", spacing++ ? " " : "", buf.version);
	if(m)
		printf("%s%s", spacing++ ? " " : "", buf.machine);
	if(spacing == 0)
		printf("%s", buf.sysname);
	printf("\n");
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "Usage: uname [-snrvma]\n\
  -s    operating system name\n\
  -n    name of this node on the network\n\
  -r    operating system release name\n\
  -v    operating system version\n\
  -m    hardware type\n\
  -a    all the options above\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int flagm = 0;
	int flagn = 0;
	int flagr = 0;
	int flags = 0;
	int flagv = 0;
	int o;

	while((o = getopt(argc, argv, "amnrsv")) != -1)
		switch(o)
		{
			case 'a':
				flagm = 1;
				flagn = 1;
				flagr = 1;
				flags = 1;
				flagv = 1;
				break;
			case 'm':
				flagm = 1;
				break;
			case 'n':
				flagn = 1;
				break;
			case 'r':
				flagr = 1;
				break;
			case 's':
				flags = 1;
				break;
			case 'v':
				flagv = 1;
				break;
			default:
				return _usage();
		}
	return _uname(flagm, flagn, flagr, flags, flagv);
}
