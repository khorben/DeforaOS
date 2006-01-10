/* vfs.c */



#include <System.h>
#include <stdio.h>


/* VFS */
static int _vfs_error(char * message, int ret);
static int _vfs(char * root)
{
	Event * event;
	AppServer * appserver;

	if((event = event_new()) == NULL)
		return _vfs_error("Event", 1);
	if((appserver = appserver_new_event("VFS", ASO_LOCAL, event)) == NULL)
	{
		_vfs_error("AppServer", 1);
		event_delete(event);
		return 1;
	}
	event_loop(event);
	appserver_delete(appserver);
	event_delete(event);
	return 0;
}

static int _vfs_error(char * message, int ret)
{
	fprintf(stderr, "%s", "VFS: ");
	perror(message);
	return ret;
}


/* close */
/* int close(int fildes)
{
	fprintf(stderr, "VFS: close(%d)\n", fildes);
	return 0;
} */


/* open */
/* int open(const char * filename, int flags)
{
	fprintf(stderr, "VFS: open(%s, %d)\n", filename, flags);
	return 0;
} */


/* main */
int main(int argc, char * argv[])
{
	return _vfs("/") == 0 ? 0 : 2;
}
