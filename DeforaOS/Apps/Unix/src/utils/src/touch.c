/* touch.c */



#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <utime.h>
#include <errno.h>


/* Prefs */
#define PREFS_a 1
#define PREFS_m 2
#define PREFS_c 4
#define PREFS_r 8
#define PREFS_t 16
typedef struct _Prefs
{
	int flags;
	char * time;
	time_t ttime;
} Prefs;

static int _prefs_ttime(char * string, time_t * time);
static int _prefs_parse(Prefs * prefs, int argc, char * argv[])
{
	int o;

	memset(prefs, 0, sizeof(Prefs));
	while((o = getopt(argc, argv, "acmr:t:")) != -1)
	{
		switch(o)
		{
			case 'a':
				prefs->flags |= PREFS_a;
				break;
			case 'c':
				prefs->flags |= PREFS_c;
				break;
			case 'm':
				prefs->flags |= PREFS_m;
				break;
			case 'r':
				prefs->flags -= prefs->flags & PREFS_t;
				prefs->flags |= PREFS_r;
				prefs->time = optarg;
				break;
			case 't':
				prefs->flags -= prefs->flags & PREFS_r;
				prefs->flags |= PREFS_t;
				if(!_prefs_ttime(optarg, &prefs->ttime))
					return 1;
				break;
			default:
				return 1;
		}
	}
	if(!((prefs->flags & PREFS_a) && (prefs->flags & PREFS_m)))
		prefs->flags |= (PREFS_a | PREFS_m);
	return 0;
}

static int _ttime_century(char ** p, time_t * time);
static int _ttime_year(char ** p, time_t * time);
static int _ttime_month(char ** p, time_t * time);
static int _ttime_day(char ** p, time_t * time);
static int _ttime_hour(char ** p, time_t * time);
static int _ttime_minut(char ** p, time_t * time);
static int _ttime_second(char ** p, time_t * time);
static int _prefs_ttime(char * string, time_t * time)
{
	time_t t = 0;
	char ** p = &string;
	int ret = 0;
	int len;

	switch((len = strlen(string)))
	{
		case 15:
		case 12:
			ret +=_ttime_century(p, &t);
		case 13:
		case 10:
			ret += _ttime_year(p, &t);
		case 11:
		case 8:
			ret += _ttime_month(p, &t);
			ret += _ttime_day(p, &t);
			ret += _ttime_hour(p, &t);
			ret += _ttime_minut(p, &t);
			if(len % 2)
				ret += _ttime_second(p, &t);
			break;
		default:
			return 1;
	}
	if(ret != 0)
		return 1;
	*time = t;
	return 0;
}

static int _ttime_number(char ** p, unsigned int * res);
static int _ttime_century(char ** p, time_t * time)
{
	/* FIXME */
	return 1;
}

static int _ttime_number(char ** p, unsigned int * res)
{
	if(**p >= '0' && **p <= '9')
	{
		*res = (**p - '0') * 10;
		(*p)++;
		if(**p >= '0' && **p <= '9')
		{
			*res += (**p - '0');
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %u\n", *res);
#endif
			(*p)++;
			return 0;
		}
	}
	return 1;
}

static int _ttime_year(char ** p, time_t * time)
{
	int res;

	if(_ttime_number(p, &res) != 0)
		return 1;
	*time = res * 60 * 60 * 24 * 365; /* FIXME */
	return 0;
}

static int _ttime_month(char ** p, time_t * time)
{
	int res;

	if(_ttime_number(p, &res) != 0)
		return 1;
	*time = res * 60 * 60 * 24 * 30; /* FIXME */
	return 0;
}

static int _ttime_day(char ** p, time_t * time)
{
	int res;

	if(_ttime_number(p, &res) != 0)
		return 1;
	*time = res * 60 * 60 * 24;
	return 0;
}

static int _ttime_hour(char ** p, time_t * time)
{
	int res;

	if(_ttime_number(p, &res) != 0)
		return 1;
	*time += res * 60 * 60;
	return 0;
}

static int _ttime_minut(char ** p, time_t * time)
{
	unsigned int res;

	if(_ttime_number(p, &res) != 0 || res >= 60)
		return 1;
	*time += res * 60;
	return 0;
}

static int _ttime_second(char ** p, time_t * time)
{
	unsigned int res;

	if(**p != '.')
		return 1;
	(*p)++;
	if(_ttime_number(p, &res) != 0 || res >= 60)
		return 1;
	*time += res;
	return 0;
}


/* touch */
static int _touch_error(char * message, int ret);
static int _touch_rtime(char * filename, time_t * atime, time_t * mtime);
static int _touch_do(Prefs * prefs, char * filename,
		time_t atime, time_t mtime);
static int _touch(Prefs * prefs, int argc, char * argv[])
{
	int res = 0;
	time_t atime = prefs->ttime;
	time_t mtime = prefs->ttime;
	int i;

	if(prefs->flags & PREFS_r)
	{
		if(_touch_rtime(prefs->time, &atime, &mtime) != 0)
			return 2;
	}
	else if(!(prefs->flags & PREFS_t))
	{
		atime = time(NULL);
		mtime = atime;
	}
	for(i = 0; i < argc; i++)
		res += _touch_do(prefs, argv[i], atime, mtime);
	return res > 0 ? 2 : 0;
}

static int _touch_error(char * message, int ret)
{
	fprintf(stderr, "%s", "touch: ");
	perror(message);
	return ret;
}

static int _touch_rtime(char * filename, time_t * atime, time_t * mtime)
{
	struct stat st;

	if(stat(filename, &st) != 0)
		return _touch_error(filename, 1);
	*atime = st.st_atime;
	*mtime = st.st_mtime;
	return 0;
}


static int _touch_do(Prefs * prefs, char * filename, time_t atime, time_t mtime)
{
	struct stat st;
	struct utimbuf ut;
	int fd;

#ifdef DEBUG
	fprintf(stderr, "%s%d%s%s%s%ld%s%ld%s", "_touch_do(", prefs->flags,
			", ", filename, ", ", atime, ", ", mtime, ");\n");
#endif
	if(!(prefs->flags & PREFS_c))
	{
		if((fd = open(filename, 0666, O_CREAT) == -1))
			_touch_error(filename, 0);
		else if(close(fd) != 0)
			_touch_error(filename, 0);
	}
	if(prefs->flags == PREFS_m || prefs->flags == PREFS_a)
		if(stat(filename, &st) != 0)
		{
			if(prefs->flags == PREFS_m)
				atime = st.st_atime;
			else
				mtime = st.st_mtime;
		}
	ut.actime = atime;
	ut.modtime = mtime;
	if(utime(filename, &ut) != 0)
	{
		if((prefs->flags & PREFS_c) && errno == ENOENT)
			return 0;
		return _touch_error(filename, 1);
	}
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: touch [-acm][-r ref_file|-t time]\
file...\n\
  -a	change the access time\n\
  -c	do not create file if it doesn't exist\n\
  -m	change the modification time\n\
  -r	use the time of the given file\n\
  -t	use the specified time as [[CC]YY]MMDDhhmm[.SS]\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;

	if(_prefs_parse(&prefs, argc, argv) != 0 || argc - optind == 0)
		return _usage();
	return _touch(&prefs, argc - optind, &argv[optind]);
}
