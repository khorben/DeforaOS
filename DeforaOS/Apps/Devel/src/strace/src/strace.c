/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel strace */
/* strace is not free software; you can redistribute it and/or modify it under
 * the terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * strace is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with strace; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <stdio.h>
#ifdef __linux__
# include "linux.h"
#endif


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
	int size = sizeof(_syscall) / 4;

	if(!WIFSTOPPED(status))
		return -1;
	switch(WSTOPSIG(status))
	{
		case SIGTRAP:
			ptrace(PTRACE_GETREGS, pid, NULL, &context);
			if(size >= context.regs.orig_eax)
				fprintf(stderr, "%s();\n",
						_syscall[context.regs.orig_eax - 1]);
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
