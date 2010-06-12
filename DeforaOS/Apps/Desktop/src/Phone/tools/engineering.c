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
	char const * answers[] =
	{
		"%EM: 6",
		"763,766,771,778,780,773",
		"20,8,14,18,17,8",
		"20,8,14,18,17,8",
		"73,46,54,51,13,12",
		"6753,10951,3531,0,1933,1924",
		"100,100,300,300,300,100",
		"80229,961995,0,1166923,0,2715553",
		"3504,724,0,3724,4,1112",
		"0,0,0,0,0,0",
		"0,0,0,0,0,0",
		"2,2,2,2,2,2",
		"0,0,0,0,0,0",
		"0,0,0,0,0,0",
		"0,0,0,0,0,0",
		"10,10,10,10,10,10",
		NULL
	};
	size_t i;

	for(i = 0; answers[i] != NULL; i++)
		if(_on_engineering_trigger_em(&plugin, answers[i]) != 0)
			error_print("engineering");
	return 0;
}


/* helper_register_trigger */
static int _helper_register_trigger(Phone * phone, PhonePlugin * plugin,
		char const * trigger, PhoneTriggerCallback callback)
{
	return 0;
}


/* main */
int main(int argc, char * argv[])
{
	PhonePluginHelper helper;
	Config * config;

	gtk_init(&argc, &argv);
	config = config_new();
	config_load(config, "/home/khorben/.phone"); /* FIXME hardcoded */
	helper.phone = (Phone *)config;
	helper.config_get = _helper_config_get;
	helper.queue = _helper_queue;
	helper.register_trigger = _helper_register_trigger;
	plugin.helper = &helper;
	_engineering_init(&plugin);
	gtk_main();
	_engineering_destroy(&plugin);
	config_delete(config);
	return 0;
}
