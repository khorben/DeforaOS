/* $Id$ */



#include <System.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "System/Parser/XML.h"

#ifdef DEBUG
# define DEBUG_CALLBACK() fprintf(stderr, "DEBUG: %s()\n", __func__)
#else
# define DEBUG_CALLBACK()
#endif


/* XML */
/* private */
/* types */
typedef enum _XMLContext
{
	XML_CONTEXT_DATA,
	XML_CONTEXT_TAG,
	XML_CONTEXT_TAG_ATTRIBUTES,
	XML_CONTEXT_TAG_ATTRIBUTES_VALUE
} XMLContext;
#define XML_CONTEXT_TAG_FIRST	XML_CONTEXT_TAG
#define XML_CONTEXT_TAG_LAST	XML_CONTEXT_TAG_ATTRIBUTES_VALUE

struct _XML
{
	XMLDocument * document;

	/* parsing */
	Parser * parser;
	XMLContext context;
};

typedef enum _XMLCode
{
	XML_CODE_DATA,
	XML_CODE_TAG_ATTRIBUTE,
	XML_CODE_TAG_ATTRIBUTE_VALUE,
	XML_CODE_TAG_CLOSE,
	XML_CODE_TAG_ENTER,
	XML_CODE_TAG_LEAVE,
	XML_CODE_TAG_NAME
} XMLCode;


/* prototypes */
/* callbacks */
static int _xml_callback_data(Parser * parser, Token * token, int c,
		void * data);
static int _xml_callback_tag_attribute(Parser * parser, Token * token, int c,
		void * data);
static int _xml_callback_tag_attribute_value(Parser * parser, Token * token,
		int c, void * data);
static int _xml_callback_tag_close(Parser * parser, Token * token, int c,
		void * data);
static int _xml_callback_tag_enter(Parser * parser, Token * token, int c,
		void * data);
static int _xml_callback_tag_leave(Parser * parser, Token * token, int c,
		void * data);
static int _xml_callback_tag_name(Parser * parser, Token * token, int c,
		void * data);
static int _xml_callback_tag_whitespace(Parser * parser, Token * token, int c,
		void * data);

/* document */
static XMLDocument * _xml_document_new(XMLNode * node);
static void _xml_document_delete(XMLDocument * document);

/* node */
static XMLNode * _xml_node_new_data(XMLNode * parent, char * data);
static void _xml_node_delete(XMLNode * node);


/* public */
/* functions */
/* xml_new */
XML * xml_new(char const * pathname)
{
	XML * xml;

	if((xml = object_new(sizeof(*xml))) == NULL)
		return NULL;
	xml->document = NULL;
	xml->parser = parser_new(pathname);
	xml->context = XML_CONTEXT_DATA;
	if(xml->parser == NULL)
	{
		xml_delete(xml);
		return NULL;
	}
	/* FIXME optionally filter out whitespaces (and comments?) */
	parser_add_callback(xml->parser, _xml_callback_tag_whitespace, xml);
	parser_add_callback(xml->parser, _xml_callback_tag_attribute, xml);
	parser_add_callback(xml->parser, _xml_callback_tag_attribute_value,
			xml);
	parser_add_callback(xml->parser, _xml_callback_tag_close, xml);
	parser_add_callback(xml->parser, _xml_callback_tag_enter, xml);
	parser_add_callback(xml->parser, _xml_callback_tag_leave, xml);
	parser_add_callback(xml->parser, _xml_callback_tag_name, xml);
	parser_add_callback(xml->parser, _xml_callback_data, xml);
	return xml;
}


/* xml_delete */
void xml_delete(XML * xml)
{
	if(xml->document != NULL)
		_xml_document_delete(xml->document);
	if(xml->parser != NULL)
		parser_delete(xml->parser);
	object_delete(xml);
}


/* accessors */
/* xml_get_document */
XMLDocument * xml_get_document(XML * xml)
{
	Token * token = NULL;

	if(xml->document != NULL)
		return xml->document;
	if((xml->document = _xml_document_new(NULL)) == NULL)
		return NULL;
	while(parser_get_token(xml->parser, &token) == 0 && token != NULL)
	{
#ifdef DEBUG
		printf("code: %u, string: \"%s\"\n", token_get_code(token),
				token_get_string(token));
#endif
		/* FIXME implement */
	}
	return xml->document;
}


/* private */
/* functions */
/* callbacks */
/* xml_callback_data */
static int _xml_callback_data(Parser * parser, Token * token, int c,
		void * data)
{
	XML * xml = data;
	char * string = NULL;
	size_t len = 0;
	char * p;

	if(xml->context != XML_CONTEXT_DATA)
		return 1;
	while(c != EOF && c != '<')
	{
		if((p = realloc(string, len + 2)) == NULL)
			return 1; /* XXX report error */
		string = p;
		string[len++] = c;
		c = parser_scan_filter(parser);
	}
	if(len == 0)
		return 1;
	DEBUG_CALLBACK();
	token_set_code(token, XML_CODE_DATA);
	string[len] = '\0';
	token_set_string(token, string);
	free(string);
	return 0;
}


/* xml_callback_tag_attribute */
static int _xml_callback_tag_attribute(Parser * parser, Token * token, int c,
		void * data)
{
	XML * xml = data;
	char * string = NULL;
	size_t len = 0;
	char * p;

	if(xml->context != XML_CONTEXT_TAG_ATTRIBUTES || isspace(c))
		return 1;
	while(c != EOF && isalnum(c))
	{
		if((p = realloc(string, len + 2)) == NULL)
			return 1; /* XXX report error */
		string = p;
		string[len++] = c;
		c = parser_scan_filter(parser);
	}
	if(len == 0)
		return 1;
	DEBUG_CALLBACK();
	token_set_code(token, XML_CODE_TAG_ATTRIBUTE);
	string[len] = '\0';
	token_set_string(token, string);
	free(string);
	if(c == '=')
		xml->context = XML_CONTEXT_TAG_ATTRIBUTES_VALUE;
	return 0;
}


/* xml_callback_tag_attribute_value */
static int _xml_callback_tag_attribute_value(Parser * parser, Token * token,
		int c, void * data)
{
	XML * xml = data;
	int q = '\0';
	char * string = NULL;
	size_t len = 0;
	char * p;

	if(xml->context != XML_CONTEXT_TAG_ATTRIBUTES_VALUE)
		return 1;
	if(c != '=')
		return 1;
	DEBUG_CALLBACK();
	if((c = parser_scan_filter(parser)) == '\'' || c == '"')
	{
		q = c;
		c = parser_scan_filter(parser);
	}
	while(c != EOF && ((q == '\0' && isalnum(c)) || (q != '\0' && c != q)))
	{
		if((p = realloc(string, len + 2)) == NULL)
			return 1; /* XXX report error */
		string = p;
		string[len++] = c;
		c = parser_scan_filter(parser);
	}
	if(q != '\0')
		parser_scan_filter(parser);
	token_set_code(token, XML_CODE_TAG_ATTRIBUTE_VALUE);
	string[len] = '\0';
	token_set_string(token, string);
	free(string);
	xml->context = XML_CONTEXT_TAG_ATTRIBUTES;
	return 0;
}


/* xml_callback_tag_close */
static int _xml_callback_tag_close(Parser * parser, Token * token, int c,
		void * data)
{
	XML * xml = data;

	if(c != '/')
		return 1;
	if(xml->context < XML_CONTEXT_TAG_FIRST
			|| xml->context > XML_CONTEXT_TAG_LAST)
		return 1;
	DEBUG_CALLBACK();
	parser_scan_filter(parser);
	token_set_code(token, XML_CODE_TAG_CLOSE);
	token_set_string(token, "/");
	return 0;
}


/* xml_callback_tag_enter */
static int _xml_callback_tag_enter(Parser * parser, Token * token, int c,
		void * data)
{
	XML * xml = data;

	if(xml->context != XML_CONTEXT_DATA || c != '<')
		return 1;
	DEBUG_CALLBACK();
	parser_scan_filter(parser);
	xml->context = XML_CONTEXT_TAG;
	token_set_code(token, XML_CODE_TAG_ENTER);
	token_set_string(token, "<");
	return 0;
}


/* xml_callback_tag_leave */
static int _xml_callback_tag_leave(Parser * parser, Token * token, int c,
		void * data)
{
	XML * xml = data;

	if(c != '>')
		return 1;
	if(xml->context < XML_CONTEXT_TAG_FIRST
			|| xml->context > XML_CONTEXT_TAG_LAST)
		return 1;
	DEBUG_CALLBACK();
	parser_scan_filter(parser);
	xml->context = XML_CONTEXT_DATA;
	token_set_code(token, XML_CODE_TAG_LEAVE);
	token_set_string(token, ">");
	return 0;
}


/* xml_callback_tag_name */
static int _xml_callback_tag_name(Parser * parser, Token * token, int c,
		void * data)
{
	XML * xml = data;
	char * string = NULL;
	size_t len = 0;
	char * p;

	if(xml->context != XML_CONTEXT_TAG || !isalnum(c))
		return 1;
	DEBUG_CALLBACK();
	do
	{
		if((p = realloc(string, len + 2)) == NULL)
			return 1; /* XXX report error */
		string = p;
		string[len++] = c;
	}
	while((c = parser_scan_filter(parser)) != EOF && c != '<' && c != '!'
			&& c != '?' && c != '/' && c != '=' && c != '>'
			&& !isspace(c));
	token_set_code(token, XML_CODE_TAG_ATTRIBUTE);
	string[len] = '\0';
	token_set_string(token, string);
	free(string);
	xml->context = XML_CONTEXT_TAG_ATTRIBUTES;
	return 0;
}


/* xml_callback_tag_whitespace */
static int _xml_callback_tag_whitespace(Parser * parser, Token * token, int c,
		void * data)
{
	XML * xml = data;

	if(!isspace(c))
		return 1;
	if(xml->context < XML_CONTEXT_TAG_FIRST
			|| xml->context > XML_CONTEXT_TAG_LAST)
		return 1;
	DEBUG_CALLBACK();
	while(isspace(c))
		c = parser_scan_filter(parser);
	if(xml->context == XML_CONTEXT_TAG_ATTRIBUTES_VALUE)
		xml->context = XML_CONTEXT_TAG_ATTRIBUTES;
	return 1;
}


/* document */
/* xml_document_new */
static XMLDocument * _xml_document_new(XMLNode * node)
{
	XMLDocument * document;

	if((document = object_new(sizeof(*document))) == NULL)
		return NULL;
	document->root = node;
	return document;
}


/* xml_document_delete */
static void _xml_document_delete(XMLDocument * document)
{
	if(document->root != NULL)
		_xml_node_delete(document->root);
	object_delete(document);
}


/* node */
/* xml_node_new_data */
static XMLNode * _xml_node_new_data(XMLNode * parent, char * data)
{
	XMLNode * ret;
	XMLNodeData * node;

	if((ret = object_new(sizeof(*node))) == NULL)
		return NULL;
	node = &ret->data;
	node->type = XML_NODE_TYPE_DATA;
	node->parent = parent;
	node->data = strdup(data);
	if(node->data == NULL)
	{
		_xml_node_delete(ret);
		return NULL;
	}
	return ret;
}


/* xml_node_delete */
static void _xml_node_delete(XMLNode * node)
{
	switch(node->type)
	{
		case XML_NODE_TYPE_DATA:
			free(node->data.data);
			break;
		case XML_NODE_TYPE_TAG:
			free(node->tag.name);
			/* FIXME delete the rest */
			break;
	}
	object_delete(node);
}
