/* id.c */



#include <unistd.h>
extern int optind;
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* print_gid */
static struct group * print_gid(gid_t gid)
{
	struct group * group;

	if((group = getgrgid(gid)) == NULL)
	{
		printf("\n");
		perror("getgrgid");
	}
	else
		printf("gid=%u(%s)", group->gr_gid, group->gr_name);
	return group;
}


/* id_all */
static int id_all(char * user)
{
	struct passwd * passwd;
	struct group * group;
	char ** p;

	if(user == NULL)
	{
		if((passwd = getpwuid(getuid())) == NULL)
		{
			printf("\n");
			perror("geteuid");
			return 2;
		}
		printf("uid=%u(%s) ", passwd->pw_uid, passwd->pw_name);
		if((group = print_gid(passwd->pw_gid)) == NULL)
			return 2;
		if((user = strdup(group->gr_name)) == NULL)
		{
			printf("\n");
			perror("strdup");
			return 2;
		}
		if(geteuid() != getuid())
		{
			if((passwd = getpwuid(geteuid())) == NULL)
			{
				printf("\n");
				perror("geteuid");
				return 2;
			}
			printf(" euid=%u(%s) e", passwd->pw_uid,
					passwd->pw_name);
			if(print_gid(passwd->pw_gid) == NULL)
				return 2;
		}
	}
	else
	{
		if((passwd = getpwnam(user)) == NULL)
		{
			perror("getpwnam");
			return 2;
		}
		printf("uid=%u(%s) ", passwd->pw_uid, passwd->pw_name);
		if((group = print_gid(passwd->pw_gid)) == NULL)
			return 2;
		if((user = strdup(group->gr_name)) == NULL)
		{
			printf("\n");
			perror("strdup");
			return 2;
		}
	}
	printf("%s%u(%s)", " groups=", passwd->pw_gid, user);
	setgrent();
	for(group = getgrent(); group != NULL; group = getgrent())
		for(p = group->gr_mem; *p != NULL; p++)
			if(strcmp(user, *p) == 0)
				printf(",%u(%s)", group->gr_gid,
						group->gr_name);
	printf("\n");
	endgrent();
	free(user);
	return 0;
}


/* id_G */
static int id_G(char * user, int flagn)
{
	struct group * group;
	char ** p;

	if(user == NULL)
	{
		if((group = getgrgid(getegid())) == NULL)
		{
			perror("getgrgid");
			return 2;
		}
		if(getegid() != getgid())
		{
			if(flagn == 0)
				printf("%u %u", getegid(), getgid());
			else
			{
				printf("%s", group->gr_name);
				if((group = getgrgid(getgid())) == NULL)
				{
					printf("\n");
					perror("getgrgid");
					return 2;
				}
				printf(" %s", group->gr_name);
			}
		}
		else
		{
			if(flagn == 0)
				printf("%u", getgid());
			else
				printf("%s", group->gr_name);
		}
	}
	else
	{
		if((group = getgrnam(user)) == NULL)
		{
			perror("getgrnam");
			return 2;
		}
		if(flagn == 0)
			printf("%u", group->gr_gid);
		else
			printf("%s", group->gr_name);
	}
	if((user = strdup(group->gr_name)) == NULL)
	{
		perror("strdup");
		return 2;
	}
	setgrent();
	for(group = getgrent(); group != NULL; group = getgrent())
	{
		for(p = group->gr_mem; *p != NULL; p++)
		{
			if(strcmp(user, *p) == 0)
			{
				if(flagn == 0)
					printf(" %u", group->gr_gid);
				else
					printf(" %s", group->gr_name);
			}
		}
	}
	printf("\n");
	endgrent();
	free(user);
	return 0;
}


/* id_g */
static int id_g(char * user, int flagn, int flagr)
{
	struct group * group;

	if(user == NULL)
	{
		if(flagn == 0)
		{
			printf("%u\n", flagr ? getegid() : getgid());
			return 0;
		}
		if((group = getgrgid(flagr ? getegid() : getgid())) == NULL)
		{
			perror("getgrgid");
			return 2;
		}
		printf("%s\n", group->gr_name);
		return 0;
	}
	if((group = getgrnam(user)) == NULL)
	{
		perror("getgrnam");
		return 2;
	}
	if(flagn == 0)
		printf("%u\n", group->gr_gid);
	else
		printf("%s\n", group->gr_name);
	return 0;
}


/* id_u */
static int id_u(char * user, int flagn, int flagr)
{
	struct passwd * passwd;

	if(user == NULL)
	{
		if(flagn == 0)
		{
			printf("%u\n", flagr ? geteuid() : getuid());
			return 0;
		}
		if((passwd = getpwuid(flagr ? geteuid() : getuid())) == NULL)
		{
			perror("getpwuid");
			return 2;
		}
		printf("%s\n", passwd->pw_name);
		return 0;
	}
	if((passwd = getpwnam(user)) == NULL)
	{
		perror("getpwnam");
		return 2;
	}
	if(flagn == 0)
		printf("%u\n", passwd->pw_uid);
	else
		printf("%s\n", passwd->pw_name);
	return 0;
}


/* id */
int id(char * user, int flag, int flagn, int flagr)
{
	if(flag == 'G')
		return id_G(user, flagn);
	if(flag == 'g')
		return id_g(user, flagn, flagr);
	if(flag == 'u')
		return id_u(user, flagn, flagr);
	return id_all(user);
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: id [-Ggu][-nr] [user]\n\
  -G    output all different group IDs\n\
  -g    output only the effective group ID\n\
  -u    output only the effective user ID\n\
  -n    output the name as a string\n\
  -r    output the real ID instead of the effective ID\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	int flag = 0;
	int flagn = 0;
	int flagr = 0;

	while((o = getopt(argc, argv, "Ggunr")) != -1)
	{
		switch(o)
		{
			case 'G':
			case 'g':
			case 'u':
				flag = o;
				break;
			case 'n':
				flagn = 1;
				break;
			case 'r':
				flagr = 1;
				break;
			case '?':
				return _usage();
		}
	}
	if(optind == argc)
		return id(NULL, flag, flagn, flagr);
	if(optind + 1 == argc)
		return id(argv[optind], flag, flagn, flagr);
	return _usage();
}
