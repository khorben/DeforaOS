/* locale.c */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


/* prefs */
typedef int Prefs;
enum { PREFS_a = 1, PREFS_m = 2, PREFS_c = 4, PREFS_k = 8 };


/* locale */
static int _locale_locales(void);
static int _locale_charsets(void);
static int _locale_default(void);
static int _locale_do(Prefs * p, char * locale);
static int _locale(Prefs * p, int argc, char * argv[])
{
	int ret = 0;
	int i;

	if(*p & PREFS_a)
		return _locale_locales();
	if(*p & PREFS_m)
		return _locale_charsets();
	if(*p == 0 && argc == 0)
		return _locale_default() ? 2 : 0;
	for(i = 0; i < argc; i++)
		ret += _locale_do(p, argv[i]);
	return ret ? 2 : 0;
}

static int _locale_locales(void)
{
	printf("%s", "C\nPOSIX\n");
	return 0;
}

static int _locale_charsets(void)
{
	printf("%s", "ISO_10646\n");
	return 0;
}

static int _locale_default(void)
{
	char * vars[] = { "LANG", NULL };
	char ** p;
	char * e;

	for(p = vars; *p != NULL; p++)
	{
		printf("%s=", *p);
		if((e = getenv(*p)) == NULL)
			printf("\"\"\n");
		else
			printf("%s\n", e);
	}
	if((e = getenv("LC_ALL")) == NULL)
		e = "";
	printf("%s%s\n", "LC_ALL=", e);
	return 0;
}

static int _locale_do(Prefs * p, char * locale)
{
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: locale [-a | -m]\n\
      locale [-ck]\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs p = 0;
	int o;

	while((o = getopt(argc, argv, "amck")) != -1)
	{
		switch(o)
		{
			case 'a':
				p -= (p & (PREFS_m | PREFS_c | PREFS_k));
				p |= PREFS_a;
				break;
			case 'm':
				p -= (p & (PREFS_a | PREFS_c | PREFS_k));
				p |= PREFS_m;
				break;
			case 'c':
				p -= (p & (PREFS_a | PREFS_m));
				p |= PREFS_c;
				break;
			case 'k':
				p -= (p & (PREFS_a | PREFS_m));
				p |= PREFS_k;
				break;
			default:
				return _usage();
		}
	}
	return _locale(&p, argc-optind, &argv[optind]);
}
