/* prefs.h */



#ifndef __PREFS_H
# define __PREFS_H


/* types */
struct prefs
{
	int c;
	int i;
	int p;
	int s;
};

/* functions */
int prefs_parse(struct prefs * prefs, int argc, char * argv[]);

#endif /* __SH_H */
