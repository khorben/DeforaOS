/* strace.c */



#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <stdio.h>
#include "linux.h"


/* strace */
static int _strace_parent(pid_t pid);
static int _strace(char * argv[])
{
	pid_t pid;

	if((pid = fork()) == -1)
	{
		perror("fork");
		return 2;
	}
	if(pid == 0)
	{
		ptrace(PTRACE_TRACEME);
		execvp(argv[0], argv);
		fprintf(stderr, "%s", "strace: ");
		perror(argv[0]);
		return 2;
	}
	return _strace_parent(pid);
}

static int _handle(pid_t pid, int res);
static int _strace_parent(pid_t pid)
{
	int status;

	for(;;)
	{
		waitpid(pid, &status, 0);
		if(_handle(pid, status) != 0)
			return 0;
	}
}

static int _handle(pid_t pid, int status)
{
	struct user context;
	int size = sizeof(syscall) / 4;

	if(!WIFSTOPPED(status))
		return -1;
	switch(WSTOPSIG(status))
	{
		case SIGTRAP:
			ptrace(PTRACE_GETREGS, pid, NULL, &context);
			if(size >= context.regs.orig_eax)
				fprintf(stderr, "%s();\n",
						syscall[context.regs.orig_eax - 1]);
			else
				fprintf(stderr, "%ld\n", context.regs.orig_eax);
			ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
			wait(0);
			ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
			break;
		default:
			ptrace(PTRACE_CONT, pid, NULL, WSTOPSIG(status));
			break;
	}
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: strace program [argument...]\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	if(argc <= 1)
		return _usage();
	return _strace(&argv[1]);
}
