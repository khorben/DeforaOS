/* ar.c */



#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ar.h>

#define min(a, b) (a) < (b) ? (a) : (b)


/* ar */
/* types */
typedef int Prefs;
#define PREFS_d 0x01
#define PREFS_p 0x02
#define PREFS_r 0x04
#define PREFS_t 0x08
#define PREFS_x 0x10
#define PREFS_c 0x20
#define PREFS_u 0x40
#define PREFS_v 0x80


/* functions */
static int _ar_error(char const * message, int ret);
static int _ar_do(Prefs * prefs, char const * archive, int filec,
		char * filev[]);
static int _ar(Prefs * prefs, char * archive, int filec, char * filev[])
{
	/* FIXME */
	return _ar_do(prefs, archive, filec, filev);
}

static int _ar_error(char const * message, int ret)
{
	fprintf(stderr, "%s", "ar: ");
	perror(message);
	return ret;
}

static int _do_sig_check(char const * archive, FILE * fp);
static int _do_seek_next(char const * archive, FILE * fp, struct ar_hdr * hdr);
static int _do_t(Prefs * prefs, char const * archive, FILE * fp,
		struct ar_hdr * hdr);
static int _do_x(Prefs * prefs, char const * archive, FILE * fp,
		struct ar_hdr * hdr);
static int _ar_do(Prefs * prefs, char const * archive, int filec,
		char * filev[])
{
	int ret = 0;
	FILE * fp;
	struct ar_hdr hdr;
	unsigned int h;
	int i;
	char * p;

	if((fp = fopen(archive, "r")) == NULL)
		return _ar_error(archive, 1);
	if(_do_sig_check(archive, fp) != 0)
	{
		fclose(fp);
		return 1;
	}
	while(fread(&hdr, sizeof(hdr), 1, fp) == 1)
	{
		if(strncmp(ARFMAG, hdr.ar_fmag, sizeof(hdr.ar_fmag)) != 0)
		{
			fprintf(stderr, "%s%s%s", "ar: ", archive,
					": Invalid archive\n");
			ret = 1;
			break;
		}
		for(h = 0, p = hdr.ar_name; h < sizeof(hdr.ar_name); h++)
			if(p[h] == '/')
			{
				p[h] = '\0';
				break;
			}
		/* FIXME what if the string doesn't get terminated? */
		if(h == 0)
		{
			if((ret = _do_seek_next(archive, fp, &hdr)) != 0)
				break;
			continue;
		}
		for(i = 0; i < filec; i++)
			if(strcmp(hdr.ar_name, filev[i]) == 0)
				break;
		if(i > 0 && i == filec)
		{
			if((ret = _do_seek_next(archive, fp, &hdr)) != 0)
				break;
			continue;
		}
		if(*prefs & PREFS_t)
			ret = _do_t(prefs, archive, fp, &hdr);
		else if(*prefs & PREFS_x)
			ret = _do_x(prefs, archive, fp, &hdr);
		if(ret != 0)
			break;
	}
	if(ret == 0 && !feof(fp))
		_ar_error(archive, 0);
	if(fclose(fp) != 0)
		return _ar_error(archive, 1);
	return ret;
}

static int _do_sig_check(char const * archive, FILE * fp)
{
	char sig[SARMAG];

	if(fread(sig, sizeof(sig), 1, fp) != 1 && !feof(fp))
		return _ar_error(archive, 1);
	if(strncmp(ARMAG, sig, SARMAG) != 0)
	{
		fprintf(stderr, "%s%s%s", "ar: ", archive,
				": Invalid archive\n");
		return 1;
	}
	return 0;
}

static int _do_seek_next(char const * archive, FILE * fp, struct ar_hdr * hdr)
{
	int size;
	char * p;

	size = strtol(hdr->ar_size, &p, 10);
	if(hdr->ar_size[0] == '\0')
	{
		fprintf(stderr, "%s%s%s", "ar: ", archive,
				": Invalid archive\n");
		return 1;
	}
	if(fseek(fp, size, SEEK_CUR) == 0)
		return 0;
	return _ar_error(archive, 1);
}

static int _t_print_long(char const * archive, struct ar_hdr * hdr);
static int _do_t(Prefs * prefs, char const * archive, FILE * fp,
		struct ar_hdr * hdr)
{
	if(*prefs & PREFS_v)
	{
		if(_t_print_long(archive, hdr) != 0)
			return 1;
	}
	else
		printf("%s\n", hdr->ar_name);
	return _do_seek_next(archive, fp, hdr);
}

static char const * _long_mode(int mode);
static int _t_print_long(char const * archive, struct ar_hdr * hdr)
{
	int mode;
	uid_t uid;
	gid_t gid;
	size_t size;
	time_t date;
	struct tm * tm;
	/* FIXME use libc's functions if possible (locales) */
	char const * month[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	mode = strtol(hdr->ar_mode, NULL, 8);
	uid = strtol(hdr->ar_uid, NULL, 10);
	gid = strtol(hdr->ar_gid, NULL, 10);
	size = strtol(hdr->ar_size, NULL, 10);
	date = strtol(hdr->ar_date, NULL, 10);
	if((tm = gmtime(&date)) == NULL)
	{
		fprintf(stderr, "%s%s%s", "ar: ", archive,
				": Invalid archive\n");
		return 1;
	}
	printf("%s %u/%u %u %s %d %02d:%02d %d %s\n", _long_mode(mode), uid,
			gid, size, month[tm->tm_mon], tm->tm_mday, tm->tm_hour,
			tm->tm_min, tm->tm_year + 1900, hdr->ar_name);
	return 0;
}

static char const * _long_mode(int mode)
{
	static char str[11];
	int i;

	for(i = 0; i < 10; i++)
		str[i] = '-';
	if(mode & S_IRUSR)
		str[1] = 'r';
	if(mode & S_IWUSR)
		str[2] = 'w';
	if(mode & S_IXUSR)
		str[3] = 'x';
	if(mode & S_IRGRP)
		str[4] = 'r';
	if(mode & S_IWGRP)
		str[5] = 'w';
	if(mode & S_IXGRP)
		str[6] = 'x';
	if(mode & S_IROTH)
		str[7] = 'r';
	if(mode & S_IWOTH)
		str[8] = 'w';
	if(mode & S_IXOTH)
		str[9] = 'x';
	str[10] = '\0';
	return str;
}

static int _do_x(Prefs * prefs, char const * archive, FILE * fp,
		struct ar_hdr * hdr)
{
	FILE * fp2;
	int fd2;
	int mode;
	int uid;
	int gid;
	size_t size;
	int date;
	char buf[BUFSIZ];
	size_t i;

	if(*prefs & PREFS_v)
		printf("%s%s\n", "x - ", hdr->ar_name);
	if((fp2 = fopen(hdr->ar_name, "w")) == NULL)
		return _ar_error(hdr->ar_name, 1);
	fd2 = fileno(fp2);
	mode = strtol(hdr->ar_mode, NULL, 8);
	if(fchmod(fd2, mode) != 0)
		_ar_error(hdr->ar_name, 0);
	uid = strtol(hdr->ar_uid, NULL, 10);
	gid = strtol(hdr->ar_gid, NULL, 10);
	if(fchown(fd2, uid, gid) != 0)
		_ar_error(hdr->ar_name, 0);
	date = strtol(hdr->ar_date, NULL, 10);
	/* FIXME */
	size = strtol(hdr->ar_size, NULL, 10);
	while(size > 0)
	{
		i = fread(buf, sizeof(char), min(sizeof(buf), size), fp);
		if(i == 0)
		{
			fclose(fp2);
			return _ar_error(archive, 1);
		}
		if(fwrite(buf, sizeof(char), i, fp2) != i)
		{
			fclose(fp2);
			return _ar_error(hdr->ar_name, 1);
		}
		size-=i;
	}
	if(fclose(fp2) != 0)
		return _ar_error(hdr->ar_name, 1);
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: ar -d[-v] archive file...\n\
       ar -p[-v] archive file...\n\
       ar -r[-cuv] archive file...\n\
       ar -t[-v] archive file...\n\
       ar -x[-v] archive file...\n\
  -d	delete one or more files from the archive\n\
  -r	replace or add files to archive\n\
  -t	write a table of contents of archive\n\
  -u	update older files in the archive\n\
  -v	give verbose output\n\
  -x	extract all or given files from the archive\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	Prefs p = 0;

	while((o = getopt(argc, argv, "dprtcuvx")) != -1)
		switch(o)
		{
			case 'd':
				p -= p & (PREFS_p|PREFS_r|PREFS_t|PREFS_x);
				p |= PREFS_d;
				break;
			case 'p':
				p -= p & (PREFS_d|PREFS_r|PREFS_t|PREFS_x);
				p |= PREFS_u;
				break;
			case 'r':
				p -= p & (PREFS_d|PREFS_p|PREFS_t|PREFS_x);
				p |= PREFS_r;
				break;
			case 't':
				p -= p & (PREFS_d|PREFS_p|PREFS_r|PREFS_x);
				p |= PREFS_t;
				break;
			case 'x':
				p -= p & (PREFS_d|PREFS_p|PREFS_r|PREFS_t);
				p |= PREFS_x;
				break;
			case 'v':
				p |= PREFS_v;
				break;
			case '?':
				return _usage();
		}
	if(optind == argc)
		return _usage();
	return _ar(&p, argv[optind], argc - optind - 1, &argv[optind + 1])
		? 2 : 0;
}
