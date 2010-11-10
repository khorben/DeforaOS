/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix devel */
/* devel is not free software; you can redistribute it and/or modify it under
 * the terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * devel is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with devel; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <errno.h>
#include <ar.h>


/* constants */
#ifndef PACKAGE
# define PACKAGE	"ar"
#endif


/* macros */
#define min(a, b) ((a) < (b) ? (a) : (b))


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
/* private */
static int _ar_error(char const * message, int ret);
static int _do_sig_check(char const * archive, FILE * fp);
static int _do_hdr_check(char const * archive, struct ar_hdr * hdr);
static int _do_seek_next(char const * archive, FILE * fp, struct ar_hdr * hdr);
static int _ar_do_r(Prefs * prefs, char const * archive, int filec,
		char * filev[]);
static int _ar_do_tx(Prefs * prefs, char const * archive, FILE * fp, int filec,
		char * filev[]);

/* accessors */
static struct tm * _ar_get_date(struct ar_hdr * ar);
static int _ar_get_uid(struct ar_hdr * ar, uid_t * uid);
static int _ar_get_gid(struct ar_hdr * ar, gid_t * gid);
static int _ar_get_mode(struct ar_hdr * ar, mode_t * mode);
static int _ar_get_size(struct ar_hdr * ar, size_t * size);

static struct tm * _ar_get_date(struct ar_hdr * ar)
{
	struct tm * ret;
	char buf[sizeof(ar->ar_date) + 1];
	time_t date;

	if(ar->ar_date[0] == '\0')
		return NULL;
	memcpy(buf, ar->ar_date, sizeof(ar->ar_date));
	buf[sizeof(buf) - 1] = '\0';
	date = strtol(buf, NULL, 10);
	if((ret = gmtime(&date)) == NULL)
		return NULL;
	return ret;
}

static int _ar_get_uid(struct ar_hdr * ar, uid_t * uid)
{
	char buf[sizeof(ar->ar_uid) + 1];

	if(ar->ar_uid[0] == '\0')
		return 1;
	memcpy(buf, ar->ar_uid, sizeof(ar->ar_uid));
	buf[sizeof(buf) - 1] = '\0';
	*uid = strtoul(buf, NULL, 10);
	return 0;
}

static int _ar_get_gid(struct ar_hdr * ar, gid_t * gid)
{
	char buf[sizeof(ar->ar_gid) + 1];

	if(ar->ar_gid[0] == '\0')
		return 1;
	memcpy(buf, ar->ar_gid, sizeof(ar->ar_gid));
	buf[sizeof(buf) - 1] = '\0';
	*gid = strtoul(buf, NULL, 10);
	return 0;
}

static int _ar_get_mode(struct ar_hdr * ar, mode_t * mode)
{
	char buf[sizeof(ar->ar_mode) + 1];

	if(ar->ar_mode[0] == '\0')
		return 1;
	memcpy(buf, ar->ar_mode, sizeof(ar->ar_mode));
	buf[sizeof(buf) - 1] = '\0';
	*mode = strtoul(buf, NULL, 8);
	return 0;
}

static int _ar_get_size(struct ar_hdr * ar, size_t * size)
{
	char buf[sizeof(ar->ar_size) + 1];

	if(ar->ar_size[0] == '\0')
		return 1;
	memcpy(buf, ar->ar_size, sizeof(ar->ar_size));
	buf[sizeof(buf) - 1] = '\0';
	*size = strtoul(buf, NULL, 10);
	return 0;
}


/* public */
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
	fputs(PACKAGE ": ", stderr);
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
	int ret;
	FILE * fp;

	if((fp = fopen(archive, "r+")) == NULL)
	{
		if(errno != ENOENT)
			return _ar_error(archive, 1);
		if(!(*prefs & PREFS_c))
			fprintf(stderr, "%s%s%s", PACKAGE ": ", archive,
					": Creating archive\n");
		return _do_create(prefs, archive, filec, filev);
	}
	ret = _do_replace(prefs, archive, fp, filec, filev);
	if(fclose(fp) != 0)
		return _ar_error(archive, 1);
	return ret;
}

static int _create_append(Prefs * prefs, char const * archive, FILE * fp,
	       	char const * filename);
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
		if(_create_append(prefs, archive, fp, filev[i]) != 0)
			break;
	if(fclose(fp) != 0)
		return _ar_error(archive, 1);
	return i != filec;
}

static int _append_header(char const * archive, FILE * fp,
		char const * filename, FILE * fp2);
static int _create_append(Prefs * prefs, char const * archive, FILE * fp,
	       	char const * filename)
{
	FILE * fp2;
	char buf[BUFSIZ];
	size_t i;

	if(*prefs & PREFS_v)
		printf("a - %s\n", filename);
	if((fp2 = fopen(filename, "r")) == NULL)
		return _ar_error(filename, 1);
	if(_append_header(archive, fp, filename, fp2) != 0)
	{
		fclose(fp2);
		return 1;
	}
	while((i = fread(buf, sizeof(char), sizeof(buf), fp2)) > 0
			&& fwrite(buf, sizeof(char), i, fp) == i);
	if(!feof(fp2) || i > 0)
	{
		_ar_error(i > 0 ? archive : filename, 0);
		fclose(fp2);
		return 1;
	}
	if(fclose(fp2) != 0)
		return _ar_error(filename, 1);
	return 0;
}

static int _append_header(char const * archive, FILE * fp,
		char const * filename, FILE * fp2)
{
	struct ar_hdr hdr;
	struct stat st;
	char * p;

	if(fstat(fileno(fp2), &st) != 0)
		return _ar_error(filename, 1);
	if((p = strdup(filename)) == NULL)
		return _ar_error(filename, 1);
	memset(&hdr, 0, sizeof(hdr));
	strncpy(hdr.ar_name, basename(p), sizeof(hdr.ar_name) - 1);
	free(p);
	snprintf(hdr.ar_date, sizeof(hdr.ar_date), "%u", (unsigned)st.st_mtime);
	snprintf(hdr.ar_uid, sizeof(hdr.ar_uid), "%u", (unsigned)st.st_uid);
	snprintf(hdr.ar_gid, sizeof(hdr.ar_gid), "%u", (unsigned)st.st_gid);
	snprintf(hdr.ar_mode, sizeof(hdr.ar_mode), "%o", (unsigned)st.st_mode);
	snprintf(hdr.ar_size, sizeof(hdr.ar_size), "%u", (unsigned)st.st_size);
	strncpy(hdr.ar_fmag, ARFMAG, sizeof(hdr.ar_fmag));
	if(fwrite(&hdr, sizeof(hdr), 1, fp) != 1)
		return _ar_error(archive, 1);
	return 0;
}

static int _do_replace(Prefs * prefs, char const * archive, FILE * fp,
		int filec, char * filev[])
{
	struct ar_hdr hdr;
	int i;
	size_t h;

	if(_do_sig_check(archive, fp) != 0)
		return 1;
	while(fread(&hdr, sizeof(hdr), 1, fp) == 1)
	{
		if(_do_hdr_check(archive, &hdr) != 0)
			return 1;
		for(h = 0; h < sizeof(hdr.ar_name); h++)
			if(hdr.ar_name[h] == '/')
			{
				hdr.ar_name[h] = '\0';
				break;
			}
		for(i = 0; i < filec; i++)
		{
			if(strlen(filev[i]) > sizeof(hdr.ar_name))
				continue;
			/* XXX test against basename(filev[i]) instead? */
			if(strncmp(hdr.ar_name, filev[i], sizeof(hdr.ar_name))
					!= 0)
				continue;
			/* FIXME implement */
			fprintf(stderr, "%s%s%s", PACKAGE ": ", filev[i],
				       	": replacing not implemented yet\n");
			filev[i] = ""; /* XXX ugly hack */
		}
		if(_do_seek_next(archive, fp, &hdr) != 0)
			return 1;
	}
	for(i = 0; i < filec; i++)
	{
		if(strlen(filev[i]) == 0) /* XXX ugly hack */
			continue;
		if(_create_append(prefs, archive, fp, filev[i]) != 0)
			return 1;
	}
	return 0;
}

static int _do_hdr_check(char const * archive, struct ar_hdr * hdr)
{
	if(strncmp(ARFMAG, hdr->ar_fmag, sizeof(hdr->ar_fmag)) != 0)
	{
		fprintf(stderr, "%s%s%s", PACKAGE ": ", archive,
			       	": Invalid archive\n");
		return 1;
	}
	return 0;
}

static int _do_seek_next(char const * archive, FILE * fp, struct ar_hdr * hdr)
{
	size_t size;

	if(_ar_get_size(hdr, &size) != 0)
	{
		fprintf(stderr, "%s%s%s", PACKAGE ": ", archive,
				": Invalid archive\n");
		return 1;
	}
	if(fseek(fp, size, SEEK_CUR) != 0)
		return _ar_error(archive, 1);
	return 0;
}


static int _do_t(Prefs * prefs, char const * archive, FILE * fp,
		struct ar_hdr * hdr, char const * name);
static int _do_x(Prefs * prefs, char const * archive, FILE * fp,
		struct ar_hdr * hdr, char const * name);
static int _ar_do_tx(Prefs * prefs, char const * archive, FILE * fp, int filec,
		char * filev[])
{
	struct ar_hdr hdr;
	char name[sizeof(hdr.ar_name) + 1];
	unsigned int h;
	int i;
	char * p;

	while(fread(&hdr, sizeof(hdr), 1, fp) == 1)
	{
		if(_do_hdr_check(archive, &hdr) != 0)
			return 1;
		memcpy(name, hdr.ar_name, sizeof(hdr.ar_name));
		name[sizeof(name) - 1] = '\0';
		for(h = 0, p = name; h < sizeof(name); h++)
			if(p[h] == '/')
			{
				p[h] = '\0';
				break;
			}
		if(h == 0)
		{
			if(_do_seek_next(archive, fp, &hdr) != 0)
				return 1; /* FIXME error case? */
			continue;
		}
		for(i = 0; i < filec; i++)
			if(strcmp(name, filev[i]) == 0)
				break;
		if(i > 0 && i == filec)
		{
			if(_do_seek_next(archive, fp, &hdr) != 0)
				return 1;
			continue;
		}
		if(*prefs & PREFS_t)
		{
			if(_do_t(prefs, archive, fp, &hdr, name) != 0)
				return 1;
			continue;
		}
		else if(*prefs & PREFS_x)
		{
			if(_do_x(prefs, archive, fp, &hdr, name) != 0)
				return 1;
			continue;
		}
		/* FIXME clean up this so it won't have to appear */
		fputs(PACKAGE ": Not implemented yet\n", stderr);
		return 1;
	}
	return 0;
}

static int _do_sig_check(char const * archive, FILE * fp)
{
	char sig[SARMAG];

	if(fread(sig, sizeof(sig), 1, fp) != 1
			&& !feof(fp))
		return _ar_error(archive, 1);
	if(strncmp(ARMAG, sig, SARMAG) == 0)
		return 0;
	fprintf(stderr, "%s%s%s", PACKAGE ": ", archive, ": Invalid archive\n");
	return 1;
}

static int _t_print_long(char const * archive, struct ar_hdr * hdr,
		char const * name);
static int _do_t(Prefs * prefs, char const * archive, FILE * fp,
		struct ar_hdr * hdr, char const * name)
{
	if(*prefs & PREFS_v)
	{
		if(_t_print_long(archive, hdr, name) != 0)
			return 1;
	}
	else
		printf("%s\n", name);
	return _do_seek_next(archive, fp, hdr);
}

static char const * _long_mode(mode_t mode);
static int _t_print_long(char const * archive, struct ar_hdr * hdr,
		char const * name)
{
	mode_t mode;
	uid_t uid;
	gid_t gid;
	size_t size;
	struct tm * tm;
	char buf[24];

	if(_ar_get_mode(hdr, &mode) != 0
			|| _ar_get_uid(hdr, &uid) != 0
			|| _ar_get_gid(hdr, &gid) != 0
			|| _ar_get_size(hdr, &size) != 0
			|| (tm = _ar_get_date(hdr)) == NULL)
	{
		fprintf(stderr, "%s%s%s", PACKAGE ": ", archive,
				": Invalid archive\n");
		return 1;
	}
	if(strftime(buf, sizeof(buf), "%b %d %H:%M %Y", tm) == 0)
		return _ar_error("strftime", 1);
	printf("%s %u/%u %zu %s %s\n", _long_mode(mode), uid, gid, size,
			buf, name);
	return 0;
}

static char const * _long_mode(mode_t mode)
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
		struct ar_hdr * hdr, char const * name)
{
	FILE * fp2;
	int fd2;
	mode_t mode;
	uid_t uid;
	gid_t gid;
	size_t size;
	char buf[BUFSIZ];
	size_t i;

	if(_ar_get_mode(hdr, &mode) != 0
			|| _ar_get_uid(hdr, &uid) != 0
			|| _ar_get_gid(hdr, &gid) != 0
			|| _ar_get_size(hdr, &size) != 0
			|| _ar_get_date(hdr) == NULL)
	{
		fprintf(stderr, "%s%s%s", PACKAGE ": ", archive,
				": Invalid archive\n");
		return 1;
	}
	if(*prefs & PREFS_v)
		printf("%s%s\n", "x - ", name);
	if((fp2 = fopen(name, "w")) == NULL)
		return _ar_error(name, 1);
	fd2 = fileno(fp2);
	if(fchmod(fd2, mode) != 0)
		_ar_error(name, 0);
	if(fchown(fd2, uid, gid) != 0)
		_ar_error(name, 0);
	/* FIXME */
	for(; size > 0; size -= i)
	{
		if((i = fread(buf, sizeof(char), min(sizeof(buf), size), fp))
				== 0)
		{
			fclose(fp2);
			return _ar_error(archive, 1);
		}
		if(fwrite(buf, sizeof(char), i, fp2) != i)
		{
			fclose(fp2);
			return _ar_error(name, 1);
		}
	}
	if(fclose(fp2) != 0)
		return _ar_error(name, 1);
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: ar -d[-v] archive file...\n\
       ar -p[-v] archive file...\n\
       ar -r[-cuv] archive file...\n\
       ar -t[-v] archive [file...]\n\
       ar -x[-v] archive [file...]\n\
  -d	Delete one or more files from the archive\n\
  -r	Replace or add files to archive\n\
  -t	Write a table of contents of archive\n\
  -u	Update older files in the archive\n\
  -v	Give verbose output\n\
  -x	Extract all or given files from the archive\n", stderr);
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
			case 'c':
				p |= PREFS_c;
				break;
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
			default:
				return _usage();
		}
	if(!(p & (PREFS_d | PREFS_r | PREFS_t | PREFS_x))
			|| optind == argc
			|| (optind+1 >= argc && !(p & (PREFS_t | PREFS_x))))
		return _usage();
	return (_ar(&p, argv[optind], argc - optind - 1, &argv[optind + 1])
			== 0) ? 0 : 2;
}
