/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#ifndef SH_JOB_H
# define SH_JOB_H


/* types */
typedef enum _JobStatus { JS_RUNNING, JS_WAIT } JobStatus;


/* functions */
int job_add(char * command, pid_t pid, JobStatus status);
/* TODO
switch to job X
send job Y to background */
int job_list(int argc, char * argv[]);
int job_pgids(int argc, char * argv[]);
int job_status(int argc, char * argv[]);

#endif /* SH_JOB_H */
