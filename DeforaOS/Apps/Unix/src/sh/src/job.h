/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix sh */
/* sh is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * sh is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with sh; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#ifndef SH_JOB_H
# define SH_JOB_H


/* types */
typedef enum _JobStatus { JS_RUNNING, JS_WAIT } JobStatus;


/* functions */
int job_add(char * command, pid_t pid, JobStatus status);
int job_kill_status(int signum, JobStatus status);
/* TODO
switch to job X
send job Y to background */
int job_list(int argc, char * argv[]);
int job_pgids(int argc, char * argv[]);
int job_status(int argc, char * argv[]);

#endif /* SH_JOB_H */
