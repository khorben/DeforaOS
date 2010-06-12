/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Phone */
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
#include <System.h>
#include "../src/plugins/engineering.c"


/* private */
/* types */
typedef struct _PhoneEngineering
{
	Config * config;
	PhonePlugin * plugin;
	PhoneTriggerCallback * callback;
} PhoneEngineering;


/* functions */
/* helper_config_get */
static char const * _helper_config_get(Phone * phone, char const * section,
		char const * variable)
{
	Config * config = (Config *)phone;

	return config_get(config, section, variable);
}


/* helper_queue */
static int _helper_queue(Phone * phone, char const * command)
{
	PhoneEngineering * pe = (PhoneEngineering *)phone;
	char const * answers1[] =
	{
		"%EM: 860,21,21,20,24,59693,13,1,0,0,0,0,0,0,0,293,0,0,2,255",
		NULL
	};
	char const * answers2[] = { "%EM: 0,0,0,0,0,255,255,0,0,0,0", NULL };
	char const * answers3[] =
	{
		"%EM: 5",
		"825,794,838,812,982,0",
		"18,21,3,20,20,0",
		"18,21,15,20,2,0",
		"19,22,16,21,21,0",
		"28,30,28,28,30,0",
		"51433,48353,48503,51453,19913,0",
		"293,293,293,293,293,0",
		"2534077,63,232981,0,22,0",
		"2524,1420,4092,0,4384,0",
		"0,0,0,0,0,0",
		"0,0,0,0,0,0",
		"2,2,2,2,2,0",
		"0,0,0,0,0,255",
		"0,0,6,0,9,0",
		"0,0,0,0,0,0",
		"3,3,15,3,3,0",
		NULL
	};
	char const * answers4[] = { "%EM: 5,120,262,003,0", NULL };
	char const ** answers;
	size_t i;

	if(strcmp(command, "AT%EM=2,1") == 0)
		answers = answers1;
	else if(strcmp(command, "AT%EM=2,2") == 0)
		answers = answers2;
	else if(strcmp(command, "AT%EM=2,3") == 0)
		answers = answers3;
	else if(strcmp(command, "AT%EM=2,4") == 0)
		answers = answers4;
	else
		return pe->callback(pe->plugin, "ERROR");
	for(i = 0; answers[i] != NULL; i++)
		if(pe->callback(pe->plugin, answers[i]) != 0)
			error_print("engineering");
	return 0;
}


/* helper_register_trigger */
static int _helper_register_trigger(Phone * phone, PhonePlugin * plugin,
		char const * trigger, PhoneTriggerCallback callback)
{
	PhoneEngineering * pe = (PhoneEngineering *)phone;

	pe->plugin = plugin;
	pe->callback = callback;
	return 0;
}


/* main */
int main(int argc, char * argv[])
{
	PhonePluginHelper helper;
	PhoneEngineering pe;
	Phone * p;

	gtk_init(&argc, &argv);
	pe.config = config_new();
	config_load(pe.config, "/home/khorben/.phone"); /* FIXME hardcoded */
	p = &pe;
	helper.phone = p;
	helper.config_get = _helper_config_get;
	helper.queue = _helper_queue;
	helper.register_trigger = _helper_register_trigger;
	plugin.helper = &helper;
	_engineering_init(&plugin);
	gtk_main();
	_engineering_destroy(&plugin);
	config_delete(pe.config);
	return 0;
}
