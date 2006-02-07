/* ar.c */



#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <errno.h>
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
static int _do_sig_check(char const * archive, FILE * fp);
static int _ar_do_r(Prefs * prefs, char const * archive, int filec,
		char * filev[]);
static int _ar_do_tx(Prefs * prefs, char const * archive, FILE * fp, int filec,
		char * filev[]);
static int _ar(Prefs * prefs, char const * archive, int filec, char * filev[])
{
	int ret = 0;
	FILE * fp;

	if(*prefs & PREFS_r)
		return _ar_do_r(prefs, archive, filec, filev);
	if((fp = fopen(archive, "r")) == NULL)
		return _ar_error(archive, 1);
	if(_do_sig_check(archive, fp) != 0)
	{
		fclose(fp);
		return 1;
	}
	if((ret = _ar_do_tx(prefs, archive, fp, filec, filev)) == 0
			&& !feof(fp))
		_ar_error(archive, 0);
	if(fclose(fp) != 0)
		return _ar_error(archive, 1);
	return ret;
}

static int _ar_error(char const * message, int ret)
{
	fprintf(stderr, "%s", "ar: ");
	perror(message);
	return ret;
}


static int _do_create(Prefs * prefs, char const * archive, int filec,
		char * filev[]);
static int _do_replace(Prefs * prefs, char const * archive, FILE * fp,
		int filec, char * filev[]);
static int _ar_do_r(Prefs * prefs, char const * archive, int filec,
		char * filev[])
{
	FILE * fp;

	if((fp = fopen(archive, "r")) == NULL)
	{
		if(errno != ENOENT)
			return _ar_error(archive, 1);
		if(!(*prefs & PREFS_c))
			fprintf(stderr, "%s%s%s", "ar: ", archive,
					": Creating archive\n");
		return _do_create(prefs, archive, filec, filev);
	}
	return _do_replace(prefs, archive, fp, filec, filev);
}

static int _create_append(char const * archive, FILE * fp, char * filename);
static int _do_create(Prefs * prefs, char const * archive, int filec,
		char * filev[])
{
	FILE * fp;
	int i;

	if((fp = fopen(archive, "w")) == NULL)
		return _ar_error(archive, 1);
	if(fwrite(ARMAG, SARMAG, 1, fp) != 1)
	{
		fclose(fp);
		return _ar_error(archive, 1);
	}
	for(i = 0; i < filec; i++)
	{
		if(*prefs & PREFS_v)
			printf("a - %s\n", filev[i]);
		if(_create_append(archive, fp, filev[i]) != 0)
			break;
	}
	if(fclose(fp) != 0)
		_ar_error(archive, 0);
	return i != filec;
}

static int _append_header(char const * archive, FILE * fp, char * filename,
		FILE * fp2);
static int _create_append(char const * archive, FILE * fp, char * filename)
	/* FIXME filename may get truncated by basename() */
{
	FILE * fp2;
	char buf[BUFSIZ];
	size_t i;

	if((fp2 = fopen(filename, "r")) == NULL)
		return _ar_error(filename, 1);
	if(_append_header(archive, fp, filename, fp2) != 0)
		return fclose(fp2) ? 1 : 1;
	for(;;)
	{
		if((i = fread(buf, sizeof(char), sizeof(buf), fp2)) == 0)
			break;
		if(fwrite(buf, sizeof(char), i, fp) == i)
			continue;
		fclose(fp2);
		return _ar_error(archive, 1);
	}
	if(!feof(fp2))
	{
		_ar_error(filename, 0);
		return fclose(fp2) ? 1 : 1;
	}
	if(fclose(fp2) != 0)
		return _ar_error(filename, 1);
	return 0;
}

static int _append_header(char const * archive, FILE * fp, char * filename,
		FILE * fp2)
{
	struct ar_hdr hdr;
	struct stat st;

	if(fstat(fileno(fp2), &st) != 0)
		return _ar_error(filename, 1);
	strncpy(hdr.ar_name, basename(filename), sizeof(hdr.ar_name)-1);
	hdr.ar_name[sizeof(hdr.ar_name)-1] = '\0'; /* FIXME necessary? */
	snprintf(hdr.ar_date, sizeof(hdr.ar_date), "%u", st.st_mtime);
	snprintf(hdr.ar_uid, sizeof(hdr.ar_uid), "%u", st.st_uid);
	snprintf(hdr.ar_gid, sizeof(hdr.ar_gid), "%u", st.st_gid);
	snprintf(hdr.ar_mode, sizeof(hdr.ar_mode), "%o", st.st_mode);
	snprintf(hdr.ar_size, sizeof(hdr.ar_size), "%u", st.st_size);
	strncpy(hdr.ar_fmag, ARFMAG, sizeof(hdr.ar_fmag));
	if(fwrite(&hdr, sizeof(hdr), 1, fp) != 1)
		return _ar_error(archive, 1);
	return 0;
}

static int _do_replace(Prefs * prefs, char const * archive, FILE * fp,
		int filec, char * filev[])
{
	/* FIXME */
	if(fclose(fp) != 0)
		return _ar_error(archive, 1);
	return 1;
}


static int _do_seek_next(char const * archive, FILE * fp, struct ar_hdr * hdr);
static int _do_t(Prefs * prefs, char const * archive, FILE * fp,
		struct ar_hdr * hdr);
static int _do_x(Prefs * prefs, char const * archive, FILE * fp,
		struct ar_hdr * hdr);
static int _ar_do_tx(Prefs * prefs, char const * archive, FILE * fp, int filec,
		char * filev[])
{
	struct ar_hdr hdr;
	unsigned int h;
	int i;
	char * p;

	while(fread(&hdr, sizeof(hdr), 1, fp) == 1)
	{
		if(strncmp(ARFMAG, hdr.ar_fmag, sizeof(hdr.ar_fmag)) != 0)
		{
			fprintf(stderr, "%s%s%s", "ar: ", archive,
					": Invalid archive\n");
			return 1;
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
			if(_do_seek_next(archive, fp, &hdr) != 0)
				return 1; /* FIXME error case? */
			continue;
		}
		for(i = 0; i < filec; i++)
			if(strcmp(hdr.ar_name, filev[i]) == 0)
				break;
		if(i > 0 && i == filec)
		{
			if(_do_seek_next(archive, fp, &hdr) != 0)
				return 1;
			continue;
		}
		if(*prefs & PREFS_t)
		{
			if(_do_t(prefs, archive, fp, &hdr) != 0)
				return 1;
			continue;
		}
		else if(*prefs & PREFS_x)
		{
			if(_do_x(prefs, archive, fp, &hdr) != 0)
				return 1;
			continue;
		}
		fprintf(stderr, "%s", "ar: Not implemented yet\n");
		return 1;
	}
	return 0;
}

static int _do_sig_check(char const * archive, FILE * fp)
{
	char sig[SARMAG];

	if(fread(sig, sizeof(sig), 1, fp) != 1 && !feof(fp))
		return _ar_error(archive, 1);
	if(strncmp(ARMAG, sig, SARMAG) == 0)
		return 0;
	fprintf(stderr, "%s%s%s", "ar: ", archive, ": Invalid archive\n");
	return 1;
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
	static char const * month[] = { "Jan", "Feb", "Mar", "Apr", "May",
		"Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	/* FIXME the header fields should be finished or their value copied */
	mode = strtol(hdr->ar_mode, NULL, 8);
	uid = strtol(hdr->ar_uid, NULL, 10);
	gid = strtol(hdr->ar_gid, NULL, 10);
	size = strtol(hdr->ar_size, NULL, 10);
	date = strtol(hdr->ar_date, NULL, 10);
	if((tm = gmtime(&date)) == NULL)
		return fprintf(stderr, "%s%s%s", "ar: ", archive,
				": Invalid archive\n") ? 1 : 1;
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
	if(!(p & (PREFS_d | PREFS_r | PREFS_t | PREFS_x)) || optind+1 >= argc)
		return _usage();
	return _ar(&p, argv[optind], argc - optind - 1, &argv[optind + 1]) != 0
		? 2 : 0;
}
