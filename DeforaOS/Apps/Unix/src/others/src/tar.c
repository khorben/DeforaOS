/* tar.c */
/* TODO:
 * - libutils in UNIX
 * - reuse code from mkdir -p and ls -l
 * - share code with ar etc (packing framework)
 * - implement ustar format */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tar.h"

#define min(a, b) ((a) < (b)) ? (a) : (b)


/* types */
typedef int Prefs;
#define PREFS_A  0x01
#define PREFS_c  0x02
#define PREFS_v  0x04
#define PREFS_vv 0x0c
#define PREFS_x  0x10


/* tar */
static int _tar_create(Prefs * prefs, char * archive, int filec,
		char * filev[]);
static int _tar_extract(Prefs * prefs, char * archive, int filec,
		char * filev[]);
static int _tar(Prefs * prefs, char * archive, int filec, char * filev[])
{
	if(*prefs & PREFS_c)
		return _tar_create(prefs, archive, filec, filev);
	if(*prefs & PREFS_x)
		return _tar_extract(prefs, archive, filec, filev);
	return 1;
}

static int _tar_error(char * message, int ret)
{
	fprintf(stderr, "%s", "tar: ");
	perror(message);
	return ret;
}

#define _from_buffer_cpy(a) tfhb->a[sizeof(tfhb->a)-1] = '\0'; \
		tfh->a = strtol(tfhb->a, &p, 8); \
	if(*tfhb->a == '\0' || *p != '\0') return 1;
static int _tar_from_buffer(TarFileHeaderBuffer * tfhb, TarFileHeader * tfh)
{
	char * p;

	_from_buffer_cpy(mode);
	_from_buffer_cpy(owner);
	_from_buffer_cpy(group);
	_from_buffer_cpy(size); /* FIXME data type too short? */
	_from_buffer_cpy(mtime);
	for(p = tfhb->filename; *p == '/'; p++);
	/* FIXME ".." directory traversal */
	memcpy(&tfh->filename, p, tfhb->filename+sizeof(tfhb->filename)-p);
	tfh->filename[sizeof(tfh->filename)-1] = '\0';
	if(tfh->filename[strlen(tfh->filename)] == '/')
		tfh->type = FT_DIRECTORY;
	else
		tfh->type = tfhb->type;
	memcpy(&tfh->link, tfhb->link, sizeof(tfhb->link));
	tfh->link[sizeof(tfh->link)-1] = '\0';
	return 0;
}

static int _create_do(Prefs * prefs, FILE * fp, char * archive,
		char * filename);
static int _tar_create(Prefs * prefs, char * archive, int filec, char * filev[])
{
	FILE * fp;
	int i;

	if((fp = fopen(archive, "w")) == NULL)
		return _tar_error(archive, 1);
	for(i = 0; i < filec; i++)
		if(_create_do(prefs, fp, archive, filev[i]) != 0)
			break;
	fclose(fp);
	return i == filec ? 0 : 1;
}

static int _doc_normal(FILE * fp, char * archive, char * filename);
static int _create_do(Prefs * prefs, FILE * fp, char * archive, char * filename)
{
	TarFileHeaderBuffer tfhb;

	if((*prefs & PREFS_vv) == PREFS_vv)
		/* FIXME */
		printf("%s %d %d %s\n", "----------", -1, -1, filename);
	else if(*prefs & PREFS_v)
		printf("%s\n", filename);
	memset(&tfhb, 0, sizeof(tfhb));
	/* FIXME */
	if(fwrite(&tfhb, sizeof(tfhb), 1, fp) != 1)
		return _tar_error(filename, 1);
	if(fseek(fp, 512-sizeof(tfhb), SEEK_CUR) != 0)
		return _tar_error(filename, 1);
	switch(tfhb.type)
	{
		case FT_NORMAL:
		case FT_CONTIGUOUS:
			return _doc_normal(fp, archive, filename);
		default:
			return 0;
	}
}

static int _doc_normal(FILE * fp, char * archive, char * filename)
{
	FILE * fp2;
	int ret = 0;
	size_t read;
	char buf[BUFSIZ];

	if((fp2 = fopen(filename, "r")) == NULL)
		return 1;
	while((read = fread(buf, sizeof(char), sizeof(buf), fp2)) != 0)
		if(fwrite(buf, sizeof(char), read, fp) != read)
		{
			ret = _tar_error(archive, 1);
			break;
		}
	if(ret == 0 && read == 0 && !feof(fp2))
		ret = _tar_error(filename, 1);
	fclose(fp2);
	return ret;
}

static int _extract_do(Prefs * prefs, FILE * fp, char * archive,
		TarFileHeader * fh, int filec, char * filev[]);
static int _tar_extract(Prefs * prefs, char * archive, int filec,
		char * filev[])
{
	FILE * fp;
	TarFileHeaderBuffer fhdrb;
	TarFileHeader fhdr;
	size_t size;
	int ret = 0;

	if((fp = fopen(archive, "r")) == NULL)
		return _tar_error(archive, 1);
	while((size = fread(&fhdrb, sizeof(fhdrb), 1, fp)) == 1)
	{
		if(fseek(fp, 512 - sizeof(fhdrb), SEEK_CUR) != 0)
		{
			ret = _tar_error(archive, 1);
			break;
		}
		if(_tar_from_buffer(&fhdrb, &fhdr) != 0
				|| _extract_do(prefs, fp, archive, &fhdr,
					filec, filev) != 0)
		{
			ret = 1;
			break;
		}
	}
	if(ret == 0 && size == 0 && !feof(fp))
		ret = _tar_error(archive, 1);
	fclose(fp);
	return ret;
}

static void _dox_print(Prefs * prefs, TarFileHeader * fh);
static int _dox_normal(FILE * fp, char * archive, TarFileHeader * fh);
static int _dox_hardlink(TarFileHeader * fh);
static int _dox_symlink(TarFileHeader * fh);
static int _dox_char(FILE * fp, char * archive, TarFileHeader * fh);
static int _dox_block(FILE * fp, char * archive, TarFileHeader * fh);
static int _dox_directory(TarFileHeader * fh);
static int _dox_fifo(FILE * fp, char * archive, TarFileHeader * fh);
static int _dox_skip(FILE * fp, char * archive, TarFileHeader * fh);
static int _extract_do(Prefs * prefs, FILE * fp, char * archive,
		TarFileHeader * fh, int filec, char * filev[])
{
	int i;

	for(i = 0; i < filec; i++)
		if(strcmp(fh->filename, filev[i]) == 0)
			break;
	if(filec != 0 && i == filec)
		return _dox_skip(fp, archive, fh);
	_dox_print(prefs, fh);
	switch(fh->type)
	{
		case FT_NORMAL:
		case FT_CONTIGUOUS:
		case '0': /* FIXME GNU compatibility */
			return _dox_normal(fp, archive, fh);
		case FT_HARDLINK:
			return _dox_hardlink(fh);
		case FT_SYMLINK:
			return _dox_symlink(fh);
		case FT_CHAR:
			return _dox_char(fp, archive, fh);
		case FT_BLOCK:
			return _dox_block(fp, archive, fh);
		case FT_DIRECTORY:
			return _dox_directory(fh);
		case FT_FIFO:
			return _dox_fifo(fp, archive, fh);
		default:
			return _dox_skip(fp, archive, fh);
	}
}

static void _dox_print(Prefs * prefs, TarFileHeader * fh)
{
	if((*prefs & PREFS_vv) == PREFS_vv)
		/* FIXME */
		printf("%s %d %d %s\n", "----------", fh->owner, fh->group,
				fh->filename);
	else if(*prefs & PREFS_v)
		printf("%s\n", fh->filename);
}

static int _dox_normal(FILE * fp, char * archive, TarFileHeader * fh)
{
	FILE * fp2;
	size_t cnt;
	size_t read;
	char buf[BUFSIZ];

	if((fp2 = fopen(fh->filename, "w")) == NULL)
		return _tar_error(fh->filename, 1);
	for(cnt = 0; cnt < fh->size; cnt+=BUFSIZ)
	{
		read = fread(buf, sizeof(char), min(BUFSIZ, fh->size-cnt), fp);
		if(read == 0)
			return _tar_error(archive, 1);
		if(fwrite(buf, sizeof(char), read, fp2) != read)
			return _tar_error(fh->filename, 1);
	}
	fclose(fp2);
	if((cnt = fh->size % 512) != 0 && fseek(fp, 512-cnt, SEEK_CUR) != 0)
			return _tar_error(archive, 1);
	return 0;
}

static int _dox_hardlink(TarFileHeader * fh)
{
	if(link(fh->link, fh->filename) != 0)
		return _tar_error(fh->filename, 1);
	return 0;
}

static int _dox_symlink(TarFileHeader * fh)
{
	if(symlink(fh->link, fh->filename) != 0)
		return _tar_error(fh->filename, 1);
	return 0;
}

static int _dox_char(FILE * fp, char * archive, TarFileHeader * fh)
{
	/* FIXME */
	return _dox_skip(fp, archive, fh);
}

static int _dox_block(FILE * fp, char * archive, TarFileHeader * fh)
{
	/* FIXME */
	return _dox_skip(fp, archive, fh);
}

static int _dox_directory(TarFileHeader * fh)
{
	if(mkdir(fh->filename, fh->mode) != 0)
		return _tar_error(fh->filename, 1);
	return 0;
}

static int _dox_fifo(FILE * fp, char * archive, TarFileHeader * fh)
{
	if(mkfifo(fh->filename, fh->mode) != 0)
		return _tar_error(fh->filename, 1);
	return 0;
}

static int _dox_skip(FILE * fp, char * archive, TarFileHeader * fh)
{
	size_t cnt;

	if(fseek(fp, fh->size, SEEK_CUR) != 0)
		return _tar_error(archive, 1);
	if((cnt = fh->size % 512) != 0 && fseek(fp, 512-cnt, SEEK_CUR) != 0)
		return _tar_error(archive, 1);
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: tar -cvx archive [file...]\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	Prefs prefs = 0;

	while((o = getopt(argc, argv, "cvx")) != -1)
		switch(o)
		{
			case 'c':
				prefs -= prefs & PREFS_x;
				prefs |= PREFS_c;
				break;
			case 'v':
				prefs |= (prefs & PREFS_v) == PREFS_v
					? PREFS_vv : PREFS_v;
				break;
			case 'x':
				prefs -= prefs & PREFS_c;
				prefs |= PREFS_x;
				break;
			default:
				return _usage();
		}
	if(prefs == 0 || argc == optind)
		return _usage();
	return _tar(&prefs, argv[optind], argc-optind-1, &argv[optind+1]) == 0
		? 0 : 2;
}
