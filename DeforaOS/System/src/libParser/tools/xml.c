/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libParser */
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
/* FIXME:
 * - plugin-based parsing system instead?
 *   * parser_new("XML", ...) ("Config", ...) */



#include <unistd.h>
#include <stdio.h>
#include "System/Parser.h"


/* functions */
/* private */
static int _usage(void)
{
	fputs("Usage: xml filename\n", stderr);
	return 1;
}


/* public */
/* main */
static void _main_node(XMLNode * node);

int main(int argc, char * argv[])
{
	int o;
	XML * xml;
	XMLDocument * doc;

	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind + 1 != argc)
		return _usage();
	if((xml = xml_new(argv[optind])) == NULL)
		return error_print("xml");
	if((doc = xml_get_document(xml)) == NULL)
		error_print("xml");
	else
		_main_node(doc->root);
	xml_delete(xml);
	return 0;
}

static void _main_node(XMLNode * node)
{
	size_t i;

	if(node == NULL)
		return;
	switch(node->type)
	{
		case XML_NODE_TYPE_DATA:
			printf("%s", node->data.data);
			break;
		case XML_NODE_TYPE_TAG:
			printf("<%s", node->tag.name);
			for(i = 0; i < node->tag.attributes_cnt; i++)
				printf(" %s=\"%s\"",
						node->tag.attributes[i]->name,
						node->tag.attributes[i]->value);
			printf(">");
			for(i = 0; i < node->tag.childs_cnt; i++)
				_main_node(node->tag.childs[i]);
			printf("</%s>", node->tag.name);
			break;
	}
}
