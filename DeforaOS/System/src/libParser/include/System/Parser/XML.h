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



#ifndef PARSER_XML_H
# define PARSER_XML_H


/* XML */
/* public */
/* types */
typedef struct _XML XML;

typedef enum _XMLFilter
{
	XML_FILTER_NONE		= 0,
	XML_FILTER_WHITESPACE	= 1
} XMLFilter;

typedef struct _XMLPrefs
{
	int filters;
} XMLPrefs;

typedef struct _XMLAttribute
{
	char * name;
	char * value;
} XMLAttribute;

typedef union _XMLNode XMLNode;

typedef struct _XMLNodeTag XMLNodeTag;

typedef enum _XMLNodeType
{
	XML_NODE_TYPE_TAG,
	XML_NODE_TYPE_DATA
} XMLNodeType;

struct _XMLNodeTag
{
	XMLNodeType type;
	XMLNodeTag * parent;
	char * name;
	XMLAttribute ** attributes;
	size_t attributes_cnt;
	XMLNode ** childs;
	size_t childs_cnt;
};

typedef struct _XMLNodeData
{
	XMLNodeType type;
	XMLNodeTag * parent;
	char * buffer;
	size_t size;
} XMLNodeData;

union _XMLNode
{
	XMLNodeType type;
	struct
	{
		XMLNodeType type;
		XMLNodeTag * parent;
	} parent;
	XMLNodeTag tag;
	XMLNodeData data;
};

typedef struct _XMLDocument
{
	XMLNode * root;
} XMLDocument;


/* functions */
XML * xml_new(XMLPrefs * prefs, char const * pathname);
XML * xml_new_string(XMLPrefs * prefs, char const * string, size_t length);
void xml_delete(XML * xml);

/* accessors */
XMLDocument * xml_get_document(XML * xml);
char const * xml_get_filename(XML * xml);

#endif /* !PARSER_XML_H */
