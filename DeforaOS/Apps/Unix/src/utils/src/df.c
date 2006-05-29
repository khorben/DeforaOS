/* df.c */



#include <sys/statvfs.h>
#include <unistd.h>
#include <stdio.h>


/* df */
static int _df_error(char const * message, int ret);
static int _df_do(char const * file);
static int _df(int filec, char * filev[])
{
	int ret = 0;
	int i;

	/* FIXME */
	printf("%s", "Filesystem 512-blocks       Used  Available Capacity Mounted on\n");
	for(i = 0; i < filec; i++)
		ret |= _df_do(filev[i]);
	return ret;
}

static int _df_error(char const * message, int ret)
{
	fprintf(stderr, "%s", "df: ");
	perror(message);
	return ret;
}

static int _df_do(char const * file)
{
	struct statvfs f;

	if(statvfs(file, &f) != 0)
		return _df_error(file, 1);
	printf("%10s %10d %10d %10d %7d%% %s\n", "", f.f_blocks,
			f.f_blocks-f.f_bfree, f.f_bavail,
			((f.f_blocks-f.f_bfree)*100)/((f.f_blocks-f.f_bfree)
						      +f.f_bavail), "");
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: df [-k][-P|-T][file...]\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;

	while((o = getopt(argc, argv, "kPT")) != -1)
		switch(o)
		{
			case 'k':
				break;
			case 'P':
				break;
			case 'T':
				break;
			default:
				return _usage();
		}
	return _df(argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
