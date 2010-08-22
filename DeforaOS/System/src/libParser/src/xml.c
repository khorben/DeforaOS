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



#include <System.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
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
/* attribute */
static XMLAttribute * _xml_attribute_new(char const * name, char const * value);
static void _xml_attribute_delete(XMLAttribute * attribute);
static int _xml_attribute_set_value(XMLAttribute * attribute,
		char const * value);


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
static XMLNode * _xml_node_new_data(XMLNodeTag * parent, char const * data);
static XMLNode * _xml_node_new_tag(XMLNodeTag * parent, char const * name);
static void _xml_node_delete(XMLNode * node);

static char const * _xml_node_tag_get_name(XMLNodeTag * node);
static int _xml_node_tag_add_attribute(XMLNodeTag * node,
		XMLAttribute * attribute);
static int _xml_node_tag_add_child(XMLNodeTag * node, XMLNode * child);


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
	parser_add_callback(xml->parser, _xml_callback_tag_name, xml);
	parser_add_callback(xml->parser, _xml_callback_tag_attribute, xml);
	parser_add_callback(xml->parser, _xml_callback_tag_attribute_value,
			xml);
	parser_add_callback(xml->parser, _xml_callback_tag_close, xml);
	parser_add_callback(xml->parser, _xml_callback_tag_enter, xml);
	parser_add_callback(xml->parser, _xml_callback_tag_leave, xml);
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
static void _document_data(Token * token, XMLNodeTag * current);
static void _document_tag_attribute(Token * token, XMLNodeTag * node,
		XMLAttribute ** attribute);
static void _document_tag_attribute_value(Token * token,
		XMLAttribute * attribute);
static void _document_tag_name(XML * xml, Token * token, XMLNodeTag ** current,
		int close);

XMLDocument * xml_get_document(XML * xml)
{
	Token * token = NULL;
	XMLCode code;
	XMLNodeTag * current = NULL;
	XMLAttribute * attribute = NULL;
	int close = 0;

	if(xml->document != NULL)
		return xml->document;
	if((xml->document = _xml_document_new(NULL)) == NULL)
		return NULL;
	while(parser_get_token(xml->parser, &token) == 0 && token != NULL)
	{
#ifdef DEBUG
		printf("code: %u, string: \"%s\", close=%d\n", token_get_code(
					token), token_get_string(token), close);
#endif
		switch((code = token_get_code(token)))
		{
			case XML_CODE_DATA:
				_document_data(token, current);
				break;
			case XML_CODE_TAG_ATTRIBUTE:
				_document_tag_attribute(token, current,
						&attribute);
				break;
			case XML_CODE_TAG_ATTRIBUTE_VALUE:
				_document_tag_attribute_value(token, attribute);
				break;
			case XML_CODE_TAG_CLOSE:
				close = 1;
				break;
			case XML_CODE_TAG_ENTER:
				break;
			case XML_CODE_TAG_NAME:
				_document_tag_name(xml, token, &current, close);
				break;
			case XML_CODE_TAG_LEAVE:
				close = 0;
				break;
		}
	}
	return xml->document;
}

static void _document_data(Token * token, XMLNodeTag * current)
{
	XMLNode * node;

	if(current == NULL)
		return; /* XXX warn */
	node = _xml_node_new_data(current, token_get_string(token));
	_xml_node_tag_add_child(current, node);
}

static void _document_tag_attribute(Token * token, XMLNodeTag * current,
		XMLAttribute ** attribute)
{
	if((*attribute = _xml_attribute_new(token_get_string(token), NULL))
			== NULL)
		return; /* XXX warn */
	_xml_node_tag_add_attribute(current, *attribute);
}

static void _document_tag_attribute_value(Token * token,
		XMLAttribute * attribute)
{
	_xml_attribute_set_value(attribute, token_get_string(token));
}

static void _document_tag_name(XML * xml, Token * token, XMLNodeTag ** current,
		int close)
{
	XMLNode * node;
	char const * parent;

	if(close == 0)
	{
		if((node = _xml_node_new_tag(*current, token_get_string(token)))
				== NULL)
			return; /* XXX warn */
		if(*current == NULL)
			xml->document->root = node;
		else
			_xml_node_tag_add_child(*current, node);
		*current = &node->tag;
	}
	else if(*current != NULL)
	{
		parent = _xml_node_tag_get_name(*current);
		if(strcmp(parent, token_get_string(token)) == 0)
			*current = (*current)->parent;
	}
	else
		; /* XXX the document is malformed */
}


/* private */
/* functions */
/* attribute */
/* xml_attribute_new */
static XMLAttribute * _xml_attribute_new(char const * name, char const * value)
{
	XMLAttribute * attribute;

	if(name == NULL)
	{
		error_set_code(1, "%s", strerror(EINVAL));
		return NULL;
	}
	if(value == NULL)
		value = name;
	if((attribute = object_new(sizeof(*attribute))) == NULL)
		return NULL;
	attribute->name = string_new(name);
	attribute->value = string_new(value);
	if(attribute->name == NULL || attribute->value == NULL)
	{
		_xml_attribute_delete(attribute);
		return NULL;
	}
	return attribute;
}


/* xml_attribute_delete */
static void _xml_attribute_delete(XMLAttribute * attribute)
{
	string_delete(attribute->name);
	string_delete(attribute->value);
	object_delete(attribute);
}


/* xml_attribute_set_value */
static int _xml_attribute_set_value(XMLAttribute * attribute,
		char const * value)
{
	char * v;

	if((v = string_new(value)) == NULL)
		return 1;
	string_delete(attribute->value);
	attribute->value = v;
	return 0;
}


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

	if(xml->context != XML_CONTEXT_TAG_ATTRIBUTES || !isalnum(c))
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
	token_set_code(token, XML_CODE_TAG_NAME);
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

	if(node != NULL && node->type != XML_NODE_TYPE_TAG)
	{
		error_set_code(1, "%s", strerror(EINVAL));
		return NULL;
	}
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
static XMLNode * _xml_node_new_data(XMLNodeTag * parent, char const * data)
{
	XMLNode * node;

	if((node = object_new(sizeof(*node))) == NULL)
		return NULL;
	node->data.type = XML_NODE_TYPE_DATA;
	node->data.parent = parent;
	node->data.data = string_new(data);
	if(node->data.data == NULL)
	{
		_xml_node_delete(node);
		return NULL;
	}
	return node;
}


/* xml_node_new_tag */
static XMLNode * _xml_node_new_tag(XMLNodeTag * parent, char const * name)
{
	XMLNode * node;

	if((node = object_new(sizeof(*node))) == NULL)
		return NULL;
	node->tag.type = XML_NODE_TYPE_TAG;
	node->tag.parent = parent;
	node->tag.name = string_new(name);
	if(node->tag.name == NULL)
	{
		_xml_node_delete(node);
		return NULL;
	}
	return node;
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


/* xml_node_tag_get_name */
static char const * _xml_node_tag_get_name(XMLNodeTag * node)
{
	return node->name;
}


/* xml_node_tag_add_attribute */
static int _xml_node_tag_add_attribute(XMLNodeTag * node,
		XMLAttribute * attribute)
{
	XMLAttribute ** p;

	if((p = realloc(node->attributes, sizeof(*p) * (node->attributes_cnt
						+ 1))) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	node->attributes = p;
	node->attributes[node->attributes_cnt++] = attribute;
	return 0;
}


/* xml_node_tag_add_child */
static int _xml_node_tag_add_child(XMLNodeTag * node, XMLNode * child)
{
	XMLNode ** p;

	if((p = realloc(node->childs, sizeof(*p) * (node->childs_cnt + 1)))
			== NULL)
		return error_set_code(1, "%s", strerror(errno));
	node->childs = p;
	node->childs[node->childs_cnt++] = child;
	return 0;
}
