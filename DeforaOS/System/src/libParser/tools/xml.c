/* $Id$ */
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
			printf("DATA: \"%s\"\n", node->data.data);
			break;
		case XML_NODE_TYPE_TAG:
			printf("TAG \"%s\":\n", node->tag.name);
			for(i = 0; i < node->tag.nodes_cnt; i++)
				_main_node(&node->tag.nodes[i]);
			break;
	}
}
