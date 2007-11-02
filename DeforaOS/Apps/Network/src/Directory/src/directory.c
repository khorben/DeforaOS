/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <pierre.pronchery@duekin.com> */



#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <System.h>
#include "directory.h"
#include "../config.h"


/* constants */
#ifndef ETCDIR
# define ETCDIR		PREFIX "/etc"
#endif


/* Directory */
/* private */
/* types */
struct _Directory
{
	Config * config;
	AppServer * appserver;
};


/* constants */
#define DIRECTORY_CONF	ETCDIR "/Directory.conf"


/* functions */
static int _x509_from_request(char const * filename, Buffer * csr,
		Buffer * x509);


/* public */
/* functions */
/* directory_new */
static void _new_config(Directory * directory);

Directory * directory_new(Event * event)
{
	Directory * directory;

	if((directory = malloc(sizeof(*directory))) == NULL)
	{
		error_set_code(1, "%s", strerror(errno));
		return NULL;
	}
	_new_config(directory);
	directory->appserver = appserver_new_event(PACKAGE, ASO_REMOTE, event);
	if(directory->config == NULL
			|| directory->appserver == NULL)
	{
		directory_delete(directory);
		return NULL;
	}
	return directory;
}

static void _new_config(Directory * directory)
{
	char * root = NULL;

	if((directory->config = config_new()) == NULL)
		return;
	if(config_load(directory->config, DIRECTORY_CONF) != 0)
	{
		config_delete(directory->config);
		directory->config = NULL;
		return;
	}
	if((root = config_get(directory->config, "", "root")) != NULL
			&& chdir(root) == 0)
		return; /* succeeded */
	if(root != NULL)
		error_set_code(1, "%s%s%s", root, ": ", strerror(errno));
	config_delete(directory->config);
	directory->config = NULL;
}


/* directory_delete */
void directory_delete(Directory * directory)
{
	if(directory->config != NULL)
		config_delete(directory->config);
	if(directory->appserver != NULL)
		appserver_delete(directory->appserver);
	free(directory);
}


/* interface */
/* register */
uint32_t directory_register(char const * title, Buffer * csr, Buffer * x509)
{
	static const char cacert_csr[] = "/cacert.csr";
	static const char begin[] = "-----BEGIN CERTIFICATE REQUEST-----\n";
	int ret;
	struct stat st;
	size_t len;
	char * filename;

	/* validate title */
	if(*title == '\0' || *title == '.' || strchr(title, '/') != NULL)
		return 1;
	/* validate request */
	if((len = buffer_get_size(csr)) < sizeof(begin))
		return error_set_print(PACKAGE, 1, "%s", "Request too short");
	if(memcmp(buffer_get_data(csr), begin, sizeof(begin) - 1) != 0)
		return error_set_print(PACKAGE, 1, "%s", "Incorrect request");
	/* verify the title is unique */
	if(lstat(title, &st) != -1 || errno != ENOENT)
		return error_set_print(PACKAGE, 1, "%s%s%s", title, ": ",
				strerror(EEXIST));
	/* request certificate */
	if(mkdir(title, 0777) != 0)
		return error_set_print(PACKAGE, 1, "%s%s%s", title, ": ",
				strerror(errno));
	if((filename = malloc(strlen(title) + sizeof(cacert_csr))) == NULL)
		return error_set_print(PACKAGE, 1, "%s", strerror(errno));
	sprintf(filename, "%s%s", title, cacert_csr);
	ret = _x509_from_request(filename, csr, x509);
	free(filename);
	return ret;
}


/* private */
/* x509_from_request */
static void _request_child(char const * filename, int fd);
static int _request_parent(Buffer * x509, pid_t pid, int fd);

static int _x509_from_request(char const * filename, Buffer * csr,
		Buffer * x509)
{
	FILE * fp;
	int fd[2];
	pid_t pid;

	if((fp = fopen(filename, "w")) == NULL)
		return error_set_code(1, "%s%s%s", filename, strerror(errno));
	if(fwrite(buffer_get_data(csr), sizeof(char), buffer_get_size(csr), fp)
			!= buffer_get_size(csr))
	{
		fclose(fp);
		return error_set_code(1, "%s%s%s", filename, strerror(errno));
	}
	if(fclose(fp) != 0)
		return error_set_code(1, "%s%s%s", filename, strerror(errno));
	if(pipe(fd) != 0)
		return error_set_code(1, "%s%s", "pipe: ", strerror(errno));
	if((pid = fork()) == -1)
	{
		close(fd[0]);
		close(fd[1]);
		return error_set_code(1, "%s%s", "fork: ", strerror(errno));
	}
	if(pid == 0)
	{
		close(fd[0]);
		_request_child(filename, fd[1]);
		exit(0);
	}
	close(fd[1]);
	return _request_parent(x509, pid, fd[0]);
}

/* PRE	title is trusted */
static void _request_child(char const * filename, int fd)
{
	static const char openssl_cnf[] = "/openssl.cnf";
	char const * title;
	char * cnf;

	if((title = config_get(config, "", "authority")) == NULL)
		exit(error_print(PACKAGE));
	if((cnf = malloc(strlen(title) + sizeof(openssl_cnf))) == NULL)
		exit(error_set_print(PACKAGE, 1, "%s", strerror(errno)));
	if(dup2(fd, 1) == -1)
		exit(error_set_print(PACKAGE, 1, "%s%s", "dup2: ", strerror(
						errno)));
	sprintf(cnf, "%s%s", title, openssl_cnf);
	execlp("openssl", "openssl", "ca", "-config", cnf, "-extensions",
			"v3_ca", "-policy", "policy_anything", "-days", "3650",
			"-batch", "-in", filename, NULL);
	exit(error_set_print(PACKAGE, 127, "%s%s", "openssl: ", strerror(
					errno)));
}

static int _parent_do(Buffer * x509, int fd);
static int _request_parent(Buffer * x509, pid_t pid, int fd)
{
	int ret;
	pid_t p;
	int status;

	ret = _parent_do(x509, fd);
	if((p = waitpid(pid, &status, 0)) == -1)
	{
		if(ret != 0)
			return ret;
		return error_set_code(1, "%s%s", "waitpid: ", strerror(errno));
	}
#ifdef DEBUG
	if(p != pid)
		fprintf(stderr, "%s%d\n", "DEBUG: waitpid() ", p);
#endif
	if(WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return ret;
	return error_set_code(1, "%s", "Unable to generate request");
}

static int _parent_do(Buffer * x509, int fd)
{
	size_t len;
	ssize_t cnt;
	char buf[8192]; /* BUFSIZ is not enough on NetBSD (1024) */

	for(len = 0; (cnt = read(fd, &buf[len], sizeof(buf) - len - 1)) > 0;
			len += cnt);
	if(cnt < 0)
		return error_set_code(1, "%s%s", "read: ", strerror(errno));
	close(fd);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: read %zu bytes\n", len);
#endif
	if(len == sizeof(buf) - 1)
		return error_set_code(1, "%s", "Buffer overrun");
	buf[len] = '\0';
#ifdef DEBUG
	fprintf(stderr, "%s%s%s", "DEBUG: x509 \"", buf, "\"\n");
#endif
	if(buffer_set_data(x509, 0, buf, len) != 0)
		return 1;
	return 0;
}
