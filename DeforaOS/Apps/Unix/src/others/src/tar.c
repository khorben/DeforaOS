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
#include <stdint.h>
#include <string.h>

#define min(a, b) ((a) < (b)) ? (a) : (b)


/* constants */
#define TAR_BLKSIZ 512


/* types */
typedef int Prefs;
#define PREFS_A  0x01
#define PREFS_c  0x02
#define PREFS_v  0x04
#define PREFS_vv 0x0c
#define PREFS_x  0x10

typedef enum {
	FT_NORMAL = 0,
	FT_HARDLINK = 1,
	FT_SYMLINK = 2,
	FT_CHAR = 3,
	FT_BLOCK = 4,
	FT_DIRECTORY = 5,
	FT_FIFO = 6,
	FT_CONTIGUOUS = 7
} FileType;

#pragma pack(1)
typedef struct _TarFileHeaderBuffer
{
	char filename[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char checksum[8];
	uint8_t type;
	char link[100];
} TarFileHeaderBuffer;
#pragma pack()

typedef struct _TarFileHeader
{
	char filename[101];
	mode_t mode;
	uid_t uid;
	gid_t gid;
	size_t size;
	time_t mtime;
	FileType type;
	char link[101];
} TarFileHeader;


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

static void _tar_print(Prefs * prefs, TarFileHeader * fh)
{
	if((*prefs & PREFS_vv) == PREFS_vv)
		/* FIXME */
		printf("%s %u %u %s\n", "----------", (unsigned)fh->uid,
				(unsigned)fh->gid, fh->filename);
	else if(*prefs & PREFS_v)
		printf("%s\n", fh->filename);
}

#define _from_buffer_cpy(a) tfhb->a[sizeof(tfhb->a)-1] = '\0'; \
		tfh->a = strtol(tfhb->a, &p, 8); \
	if(*tfhb->a == '\0' || *p != '\0') return 1;
static int _tar_from_buffer(TarFileHeaderBuffer * tfhb, TarFileHeader * tfh)
{
	char * p;

	_from_buffer_cpy(mode);
	_from_buffer_cpy(uid);
	_from_buffer_cpy(gid);
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

static void _tar_stat_to_buffer(char * filename, struct stat * st,
		TarFileHeaderBuffer * tfhb)
{
	uint8_t * p;
	size_t i;
	int checksum = 0;

	memset(tfhb, 0, sizeof(*tfhb));
	snprintf(tfhb->filename, sizeof(tfhb->filename), "%s", filename);
	snprintf(tfhb->mode, sizeof(tfhb->mode), "%07o", (unsigned)st->st_mode);
	snprintf(tfhb->uid, sizeof(tfhb->uid), "%07o", (unsigned)st->st_uid);
	snprintf(tfhb->gid, sizeof(tfhb->gid), "%07o", (unsigned)st->st_gid);
	snprintf(tfhb->size, sizeof(tfhb->size), "%011o",
			(unsigned)st->st_size);
	snprintf(tfhb->mtime, sizeof(tfhb->mtime), "%011o",
			(unsigned)st->st_mtime);
	memset(&tfhb->checksum, ' ', sizeof(tfhb->checksum));
	if(S_ISDIR(st->st_mode))
		tfhb->type = FT_DIRECTORY;
	else if(S_ISCHR(st->st_mode))
		tfhb->type = FT_CHAR;
	else if(S_ISBLK(st->st_mode))
		tfhb->type = FT_BLOCK;
	/* FIXME link */
	p = (uint8_t*)tfhb;
	for(i = 0; i < sizeof(*tfhb); i++)
		checksum+=p[i];
	snprintf(tfhb->checksum, sizeof(tfhb->checksum), "%06o%c ", checksum,
			'\0');
}

static int _create_do(Prefs * prefs, FILE * fp, char * archive,
		char * filename);
static int _tar_create(Prefs * prefs, char * archive, int filec, char * filev[])
{
	FILE * fp = stdout;
	int i;

	if(archive != NULL && (fp = fopen(archive, "w")) == NULL)
		return _tar_error(archive, 1);
	for(i = 0; i < filec; i++)
		if(_create_do(prefs, fp, archive, filev[i]) != 0)
			break;
	if(i != filec)
	{
		fclose(fp);
		return 1;
	}
	for(i = 0; i < TAR_BLKSIZ * 2 && fputc('\0', fp) == '\0'; i++);
	if(archive != NULL)
		fclose(fp);
	return i == TAR_BLKSIZ * 2 ? 0 : 1;
}

static int _doc_header(Prefs * prefs, FILE * fp, char * archive, FILE * fp2,
		char * filename, TarFileHeaderBuffer * tfhb);
static int _doc_normal(FILE * fp, char * archive, FILE * fp2, char * filename);
static int _create_do(Prefs * prefs, FILE * fp, char * archive, char * filename)
{
	FILE * fp2;
	TarFileHeaderBuffer tfhb;
	int ret;

	if((fp2 = fopen(filename, "r")) == NULL)
		return 1;
	if(_doc_header(prefs, fp, archive, fp2, filename, &tfhb) != 0)
	{
		fclose(fp2);
		return 1;
	}
	switch(tfhb.type)
	{
		case FT_NORMAL:
		case FT_CONTIGUOUS:
			ret = _doc_normal(fp, archive, fp2, filename);
			break;
		case FT_HARDLINK:
		case FT_SYMLINK:
		case FT_CHAR:
		case FT_BLOCK:
		case FT_DIRECTORY: /* FIXME recurse */
		case FT_FIFO:
			ret = 0;
			break;
		default:
			ret = 1;
	}
	fclose(fp2);
	return ret;
}

static int _doc_header(Prefs * prefs, FILE * fp, char * archive, FILE * fp2,
		char * filename, TarFileHeaderBuffer * tfhb)
{
	TarFileHeader tfh;
	struct stat st;
	int i;

	if(fstat(fileno(fp2), &st) != 0)
		return _tar_error(filename, 1);
	_tar_stat_to_buffer(filename, &st, tfhb);
	_tar_from_buffer(tfhb, &tfh);
	_tar_print(prefs, &tfh);
	if(fwrite(tfhb, sizeof(*tfhb), 1, fp) != 1)
		return _tar_error(archive, 1);
	for(i = sizeof(*tfhb); i < TAR_BLKSIZ && fputc('\0', fp) == '\0'; i++);
	if(i != TAR_BLKSIZ)
		return _tar_error(archive, 1);
	return 0;
}

static int _doc_normal(FILE * fp, char * archive, FILE * fp2, char * filename)
{
	int ret = 0;
	size_t read;
	size_t cnt;
	char buf[BUFSIZ];

	for(cnt = 0; (read = fread(buf, sizeof(char), sizeof(buf), fp2)) != 0;
			cnt+=read)
		if(fwrite(buf, sizeof(char), read, fp) != read)
		{
			ret = _tar_error(archive, 1);
			break;
		}
	if(ret == 0 && read == 0 && !feof(fp2))
		return _tar_error(filename, 1);
	for(cnt = TAR_BLKSIZ - (cnt % TAR_BLKSIZ);
			cnt > 0 && fputc('\0', fp) == '\0'; cnt--);
	return cnt == 0 ? 0 : _tar_error(archive, 1);
}

static int _extract_do(Prefs * prefs, FILE * fp, char * archive,
		TarFileHeader * fh, int filec, char * filev[]);
static int _tar_extract(Prefs * prefs, char * archive, int filec,
		char * filev[])
{
	FILE * fp = stdin; /* FIXME breaks fseek */
	TarFileHeaderBuffer fhdrb;
	TarFileHeader fhdr;
	size_t size;
	int ret = 0;

	if(archive != NULL && (fp = fopen(archive, "r")) == NULL)
		return _tar_error(archive, 1);
	while((size = fread(&fhdrb, sizeof(fhdrb), 1, fp)) == 1)
	{
		if(fseek(fp, TAR_BLKSIZ - sizeof(fhdrb), SEEK_CUR) != 0)
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
	if(archive != NULL)
		fclose(fp);
	return ret;
}

static int _dox_normal(FILE * fp, char * archive, TarFileHeader * fh);
static int _dox_hardlink(TarFileHeader * fh);
static int _dox_symlink(TarFileHeader * fh);
static int _dox_char(FILE * fp, char * archive, TarFileHeader * fh);
static int _dox_block(FILE * fp, char * archive, TarFileHeader * fh);
static int _dox_directory(TarFileHeader * fh);
static int _dox_fifo(TarFileHeader * fh);
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
	_tar_print(prefs, fh);
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
			return _dox_fifo(fh);
		default:
			return _dox_skip(fp, archive, fh);
	}
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
	if((cnt = fh->size % TAR_BLKSIZ) != 0
			&& fseek(fp, TAR_BLKSIZ-cnt, SEEK_CUR) != 0)
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
	return 1;
}

static int _dox_block(FILE * fp, char * archive, TarFileHeader * fh)
{
	/* FIXME */
	return 1;
}

static int _dox_directory(TarFileHeader * fh)
{
	if(mkdir(fh->filename, fh->mode) != 0)
		return _tar_error(fh->filename, 1);
	return 0;
}

static int _dox_fifo(TarFileHeader * fh)
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
	if((cnt = fh->size % TAR_BLKSIZ) != 0
			&& fseek(fp, TAR_BLKSIZ-cnt, SEEK_CUR) != 0)
		return _tar_error(archive, 1);
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: tar -cvx [-f archive][file...]\n\
  -c	Create an archive\n\
  -f	Specify an archive to read from or write to (default: stdin or stdout)\n\
  -v	Verbose mode\n\
  -x	Extract from archive\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	Prefs prefs = 0;
	char * archive = NULL;

	while((o = getopt(argc, argv, "cvxf:")) != -1)
		switch(o)
		{
			case 'c':
				prefs -= prefs & PREFS_x;
				prefs |= PREFS_c;
				break;
			case 'f':
				archive = optarg;
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
	if(prefs == 0)
		return _usage();
	return _tar(&prefs, archive, argc-optind-1, &argv[optind]) == 0 ? 0 : 2;
}
