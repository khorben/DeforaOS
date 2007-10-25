/* $Id$ */



#include <unistd.h>
#include <stdio.h>
#include <System.h>
#include "directory.h"
#include "../config.h"


/* usage */
static int _usage(void)
{
	fputs("Usage: " PACKAGE "\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	Event * event;
	Directory * directory;

	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if((event = event_new()) == NULL)
		return error_print(PACKAGE) ? 2 : 2;
	if((directory = directory_new(event)) == NULL)
	{
		error_print(PACKAGE);
		event_delete(event);
		return 2;
	}
	event_loop(event);
	event_delete(event);
	return 0;
}
