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
#include <string.h>
#include "System/Parser.h"


/* functions */
/* private */
/* error */
static int _error(char const * message, int ret)
{
	fputs("xml: ", stderr);
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: xml [-w][-o output] filename\n"
"       xml -s [-w][-o output] string\n", stderr);
	return 1;
}


/* public */
/* main */
static void _main_node(XMLPrefs * prefs, FILE * fp, XMLNode * node,
		unsigned int level);

int main(int argc, char * argv[])
{
	int o;
	XMLPrefs prefs;
	XML * xml;
	XMLDocument * doc;
	char const * output = NULL;
	FILE * fp = stdout;
	char const * string = NULL;

	memset(&prefs, 0, sizeof(prefs));
	while((o = getopt(argc, argv, "o:s:w")) != -1)
		switch(o)
		{
			case 'o':
				output = optarg;
				break;
			case 's':
				string = optarg;
				break;
			case 'w':
				prefs.filters |= XML_FILTER_WHITESPACE;
				break;
			default:
				return _usage();
		}
	if(output != NULL && (fp = fopen(output, "w")) == NULL)
		return _error(output, 2);
	if(string == NULL)
	{
		if(optind + 1 != argc)
			return _usage();
		xml = xml_new(&prefs, argv[optind]);
	}
	else if(optind != argc)
		return _usage();
	else
		xml = xml_new_string(&prefs, string, strlen(string));
	if(xml == NULL)
		return error_print("xml");
	if((doc = xml_get_document(xml)) == NULL)
		error_print("xml");
	else
		_main_node(&prefs, fp, doc->root, 0);
	if(output != NULL)
		fclose(fp);
	xml_delete(xml);
	return 0;
}

static void _main_node(XMLPrefs * prefs, FILE * fp, XMLNode * node,
		unsigned int level)
{
	size_t i;

	if(node == NULL)
		return;
	switch(node->type)
	{
		case XML_NODE_TYPE_DATA:
			fwrite(node->data.buffer, 1, node->data.size, fp);
			break;
		case XML_NODE_TYPE_ENTITY:
			fprintf(fp, "&%s;", node->entity.name);
			break;
		case XML_NODE_TYPE_TAG:
			if(prefs->filters & XML_FILTER_WHITESPACE && level > 0)
			{
				fputc('\n', fp);
				for(i = 0; i < level; i++)
					fputs("  ", fp);
			}
			fprintf(fp, "<%s", node->tag.name);
			for(i = 0; i < node->tag.attributes_cnt; i++)
				fprintf(fp, " %s=\"%s\"",
						node->tag.attributes[i]->name,
						node->tag.attributes[i]->value);
			if(node->tag.childs_cnt == 0)
			{
				fputs("/>", fp);
				break;
			}
			fputs(">", fp);
			for(i = 0; i < node->tag.childs_cnt; i++)
				_main_node(prefs, fp, node->tag.childs[i],
						level + 1);
			if(prefs->filters & XML_FILTER_WHITESPACE && level == 0)
				fputc('\n', fp);
			fprintf(fp, "</%s>", node->tag.name);
			if(prefs->filters & XML_FILTER_WHITESPACE && level == 0)
				fputc('\n', fp);
			break;
	}
}
