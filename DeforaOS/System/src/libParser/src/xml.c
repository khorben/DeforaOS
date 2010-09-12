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
	XMLPrefs prefs;
	XMLDocument * document;

	/* parsing */
	Parser * parser;
	XMLContext context;
	char * inject;
};

typedef enum _XMLCode
{
	XML_CODE_DATA,
	XML_CODE_TAG_ATTRIBUTE,
	XML_CODE_TAG_ATTRIBUTE_VALUE,
	XML_CODE_TAG_CLOSE,
	XML_CODE_TAG_ENTER,
	XML_CODE_TAG_LEAVE,
	XML_CODE_TAG_NAME,
	XML_CODE_TAG_SPECIAL
} XMLCode;


/* prototypes */
static int _xml_inject(XML * xml, char const * string);

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
static int _xml_callback_tag_special(Parser * parser, Token * token, int c,
		void * data);
static int _xml_callback_tag_whitespace(Parser * parser, Token * token, int c,
		void * data);

/* document */
static XMLDocument * _xml_document_new(XMLNode * node);
static void _xml_document_delete(XMLDocument * document);

/* filters */
static int _xml_filter_comment(int * c, void * data);
static int _xml_filter_inject(int * c, void * data);
static int _xml_filter_whitespace(int * c, void * data);

/* node */
static XMLNode * _xml_node_new(XMLNodeType type, XMLNodeTag * parent);
static XMLNode * _xml_node_new_data(XMLNodeTag * parent, char const * buffer,
		size_t size);
static XMLNode * _xml_node_new_tag(XMLNodeTag * parent, char const * name);
static void _xml_node_delete(XMLNode * node);

static char const * _xml_node_tag_get_name(XMLNodeTag * node);
static int _xml_node_tag_add_attribute(XMLNodeTag * node,
		XMLAttribute * attribute);
static int _xml_node_tag_add_child(XMLNodeTag * node, XMLNode * child);


/* public */
/* functions */
/* xml_new */
static XML * _new_do(XMLPrefs * prefs, char const * pathname,
		char const * string, size_t length);

XML * xml_new(XMLPrefs * prefs, char const * pathname)
{
	return _new_do(prefs, pathname, NULL, 0);
}

static XML * _new_do(XMLPrefs * prefs, char const * pathname,
		char const * string, size_t length)
{
	XML * xml;

	if((xml = object_new(sizeof(*xml))) == NULL)
		return NULL;
	if(prefs != NULL)
		memcpy(&xml->prefs, prefs, sizeof(xml->prefs));
	else
		memset(&xml->prefs, 0, sizeof(xml->prefs));
	xml->document = NULL;
	if(pathname != NULL)
		xml->parser = parser_new(pathname);
	else
		xml->parser = parser_new_string(string, length);
	xml->context = XML_CONTEXT_DATA;
	xml->inject = NULL;
	if(xml->parser == NULL)
	{
		xml_delete(xml);
		return NULL;
	}
	parser_add_filter(xml->parser, _xml_filter_inject, xml);
	if((xml->prefs.filters & XML_FILTER_WHITESPACE)
			== XML_FILTER_WHITESPACE)
		parser_add_filter(xml->parser, _xml_filter_whitespace, xml);
	/* FIXME filter out comments only optionally */
	parser_add_filter(xml->parser, _xml_filter_comment, xml);
	parser_add_callback(xml->parser, _xml_callback_tag_whitespace, xml);
	parser_add_callback(xml->parser, _xml_callback_tag_special, xml);
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


/* xml_new_string */
XML * xml_new_string(XMLPrefs * prefs, char const * string, size_t length)
{
	return _new_do(prefs, NULL, string, length);
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
	for(; parser_get_token(xml->parser, &token) == 0 && token != NULL;
			token_delete(token))
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() code=%u string: \"%s\" close=%d\n",
				__func__, token_get_code(token),
				token_get_string(token), close);
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
			case XML_CODE_TAG_LEAVE:
				close = 0;
				break;
			case XML_CODE_TAG_NAME:
				_document_tag_name(xml, token, &current, close);
				break;
			case XML_CODE_TAG_SPECIAL:
				break;
		}
	}
	return xml->document;
}

static void _document_data(Token * token, XMLNodeTag * current)
{
	XMLNode * node;
	String const * string;
	size_t size = 0;

	if(current == NULL)
		return; /* XXX warn */
	if((string = token_get_string(token)) != NULL)
		size = string_length(string);
	node = _xml_node_new_data(current, string, size);
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


/* xml_get_filename */
char const * xml_get_filename(XML * xml)
{
	return parser_get_filename(xml->parser);
}


/* private */
/* functions */
/* xml_inject */
static int _xml_inject(XML * xml, char const * string)
{
	if(string == NULL || string[0] == '\0')
		return 0; /* don't bother */
	if(xml->inject == NULL)
	{
		if((xml->inject = string_new(string)) == NULL)
			return -1;
	}
	else if(string_append(&xml->inject, string) != 0)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\") => \"%s\"\n", __func__,
			(void*)xml, string, xml->inject);
#endif
	return 0;
}


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
	int q = '\0';
	char * string = NULL;
	size_t len = 0;
	char * p;

	if(xml->context != XML_CONTEXT_TAG_ATTRIBUTES || (!isalnum(c)
				&& c != '"'))
		return 1;
	if(c == '"')
	{
		q = c;
		c = parser_scan_filter(parser);
	}
	while(c != EOF && (isalnum(c) || c == ':' || c == '-'
				|| (q != '\0' && c != q)))
	{
		if((p = realloc(string, len + 2)) == NULL)
			return 1; /* XXX report error */
		string = p;
		string[len++] = c;
		c = parser_scan_filter(parser);
	}
	if(len == 0)
		return 1;
	if(q != '\0')
		parser_scan_filter(parser);
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
	if(len == 0)
		token_set_string(token, "");
	else
	{
		string[len] = '\0';
		token_set_string(token, string);
		free(string);
	}
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


/* xml_callback_tag_special */
/* FIXME decompose this function in at least two different ones */
static int _xml_callback_tag_special(Parser * parser, Token * token, int c,
		void * data)
{
	XML * xml = data;
	char buf[2] = { '\0', '\0' };

	if((xml->context != XML_CONTEXT_TAG && xml->context
				!= XML_CONTEXT_TAG_ATTRIBUTES)
			|| (c != '?' && c != '!'))
		return 1;
	DEBUG_CALLBACK();
	buf[0] = c;
	parser_scan_filter(parser);
	token_set_code(token, XML_CODE_TAG_SPECIAL);
	token_set_string(token, buf);
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


/* filters */
/* xml_filter_comment */
static int _xml_filter_comment(int * c, void * data)
{
	XML * xml = data;
	char start[5] = "<!--";
	size_t i;

	if(*c != '<')
		return 0;
	for(i = 1; i < sizeof(start) - 1; i++)
	{
		if((*c = parser_scan(xml->parser)) == start[i])
			continue;
		start[i] = *c;
		start[i + 1] = '\0';
		if(_xml_inject(xml, &start[1]) != 0)
			return -1;
		*c = '<';
		return 1;
	}
	for(*c = parser_scan(xml->parser), i = 0; *c != EOF;
			*c = parser_scan(xml->parser))
		if(*c == '-')
			i++;
		else if(i >= 2 && *c == '>')
			break;
		else
			i = 0;
	*c = parser_scan(xml->parser);
	return 0;
}


/* xml_filter_inject */
static int _xml_filter_inject(int * c, void * data)
{
	XML * xml = data;
	size_t len;
	int d;

	if(xml->inject == NULL)
		return 0;
	if((len = strlen(xml->inject)) > 0)
	{
		d = *c;
		*c = xml->inject[0];
		memmove(xml->inject, &xml->inject[1], len--);
	}
	if(len > 0)
		return 1;
	free(xml->inject);
	xml->inject = NULL;
	return 0;
}


/* xml_filter_whitespace */
static int _xml_filter_whitespace(int * c, void * data)
{
	XML * xml = data;
	char buf[2] = { '\0', '\0' };

	if(!isspace(*c))
		return 0;
	for(*c = parser_scan(xml->parser); isspace(*c);
			*c = parser_scan(xml->parser));
	if(*c == EOF)
		return 0;
	buf[0] = *c;
	if(_xml_inject(xml, buf) != 0)
		return -1;
	*c = ' ';
	return 1;
}


/* node */
/* xml_node_new */
static XMLNode * _xml_node_new(XMLNodeType type, XMLNodeTag * parent)
{
	XMLNode * node;

	if((node = object_new(sizeof(*node))) == NULL)
		return NULL;
	node->type = type;
	node->parent.parent = parent;
	return node;
}


/* xml_node_new_data */
static XMLNode * _xml_node_new_data(XMLNodeTag * parent, char const * buffer,
		size_t size)
{
	XMLNode * node;

	if((node = _xml_node_new(XML_NODE_TYPE_DATA, parent)) == NULL)
		return NULL;
	/* XXX use buffer_new() */
	node->data.buffer = object_new(size);
	if(node->data.buffer == NULL)
	{
		_xml_node_delete(node);
		return NULL;
	}
	memcpy(node->data.buffer, buffer, size);
	node->data.size = size;
	return node;
}


/* xml_node_new_tag */
static XMLNode * _xml_node_new_tag(XMLNodeTag * parent, char const * name)
{
	XMLNode * node;

	if((node = _xml_node_new(XML_NODE_TYPE_TAG, parent)) == NULL)
		return NULL;
	node->tag.name = string_new(name);
	node->tag.attributes = NULL;
	node->tag.attributes_cnt = 0;
	node->tag.childs = NULL;
	node->tag.childs_cnt = 0;
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
	size_t i;

	switch(node->type)
	{
		case XML_NODE_TYPE_DATA:
			object_delete(node->data.buffer);
			break;
		case XML_NODE_TYPE_TAG:
			for(i = 0; i < node->tag.attributes_cnt; i++)
				_xml_attribute_delete(node->tag.attributes[i]);
			free(node->tag.attributes);
			for(i = 0; i < node->tag.childs_cnt; i++)
				_xml_node_delete(node->tag.childs[i]);
			free(node->tag.childs);
			free(node->tag.name);
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
