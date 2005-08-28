/* init.c */



#include <System.h>
#include <stdio.h>


/* Init */
static int _init(void)
{
	Event * event;
	AppServer * appserver;

	if((event = event_new()) == NULL)
		return 1;
	if((appserver = appserver_new_event("Init", event)) == NULL)
	{
		event_delete(event);
		return 1;
	}
	for(;;)
		event_loop(event);
	return 0;
}


/* usage */
static int _init_usage(void)
{
	fprintf(stderr, "%s", "Usage: Init\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	if(argc != 1)
		return _init_usage();
	return _init() == 0 ? 0 : 2;
}
