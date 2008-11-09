/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System Init */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "service.h"
#include "common.h"
#include "session.h"


/* Session */
/* types */
struct _Session
{
	String * name;
	String * profile;
	Event * event;

	Config * config;
	Service ** services;
	size_t services_cnt;
};


/* prototypes */
/* private */
/* accessors */
static String ** _session_get_config_services(Session * session);
static String * _session_get_filename(Session * session);

/* useful */
static int _session_load(Session * session);


/* functions */
/* public */
/* session_new */
Session * session_new(char const * name, char const * profile, Event * event)
{
	Session * session;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", \"%s\", %p)\n", __func__, name,
			profile, event);
#endif
	if(name == NULL || event == NULL)
	{
		error_set_code(1, "%s", strerror(EINVAL));
		return NULL;
	}
	if((session = object_new(sizeof(*session))) == NULL)
		return NULL;
	session->name = string_new(name);
	session->profile = (profile != NULL) ? string_new(profile) : NULL;
	session->event = event;
	session->config = config_new();
	session->services = NULL;
	session->services_cnt = 0;
	/* kickoff the session */
	/* FIXME should this be really fatal? annoying for Init */
	if(session_start(session) != 0)
	{
		session_delete(session);
		return NULL;
	}
	return session;
}


/* session_delete */
void session_delete(Session * session)
{
	string_delete(session->name);
	string_delete(session->profile);
	config_delete(session->config);
	object_delete(session);
}


/* AppInterface */
/* session_register */
int session_register(String const * interface, uint16_t port)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", %u)\n", __func__, interface, port);
#endif
	/* FIXME not implemented yet */
	return -1;
	/* authenticate client */
	/* check permissions */
	/* verify conflicts */
	/* register in the services list */
	/* return 0; */
}


/* useful */
/* session_start */
int session_start(Session * session)
{
	size_t i;

	if(_session_load(session) != 0)
		return -1;
	for(i = 0; i < session->services_cnt; i++)
		if(service_start(session->services[i]) != 0)
			/* we ignore the error because it gets monitored */
			error_print(PACKAGE);
	return 0;
}


/* session_stop */
int session_stop(Session * session)
{
	size_t i;

	/* FIXME should wait until the last service has stopped */
	/* FIXME again prone to race conditions */
	for(i = 0; i < session->services_cnt; i++)
		service_delete(session->services[i]);
	session->services_cnt = 0;
	return 0;
}


/* private */
/* session_get_config_services */
static String ** _session_get_config_services(Session * session)
{
	String const * profile;
	String const * services;

	if((profile = session->profile) == NULL)
		profile = config_get(session->config, NULL, "profile");
	if((services = config_get(session->config, profile, "services"))
			== NULL)
		return NULL;
	return string_explode(services, ",");
}


/* session_get_filename */
static String * _session_get_filename(Session * session)
{
	char const * format = "%s/%s.%s";
	String * p;
	int size;

	if(session->name == NULL)
	{
		error_set_code(1, "%s", strerror(EINVAL));
		return NULL;
	}
	if((size = snprintf(NULL, 0, format, SESSIONDIR, session->name,
					SESSIONEXT)) <= 0)
	{
		error_set_code(1, "%s", strerror(EINVAL));
		return NULL;
	}
	if((p = object_new(++size)) == NULL) /* XXX string_new_format() ? */
		return NULL;
	snprintf(p, size, format, SESSIONDIR, session->name, SESSIONEXT);
	return p;
}


/* useful */
static int _load_service(Session * session, String const * service);

static int _session_load(Session * session)
{
	int ret;
	String * filename;
	String ** services;
	String ** p;

	if(session->services_cnt != 0)
		return error_set_code(1, "%s", "Session is already started");
	/* lookup configuration file */
	if((filename = _session_get_filename(session)) == NULL)
		return -1;
	config_reset(session->config);
	ret = config_load(session->config, filename);
	string_delete(filename);
	if(ret != 0)
		return -1;
	/* parse the services list */
	if((services = _session_get_config_services(session)) == NULL)
		return -1;
	/* add the services */
	for(p = services; *p != NULL; p++)
	{
		if(_load_service(session, *p) != 0)
			error_print(PACKAGE);
		string_delete(*p);
	}
	free(services);
	return 0;
}

static int _load_service(Session * session, String const * service)
{
	Service ** p;
	size_t cnt = session->services_cnt;

	if((p = realloc(session->services, sizeof(*p) * (cnt + 1))) == NULL)
		return -error_set_code(1, "%s", strerror(errno));
	session->services = p;
	if((session->services[cnt] = service_new(service)) == NULL)
		return -1;
	session->services_cnt++;
	return 0;
}
