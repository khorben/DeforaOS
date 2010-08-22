/* $Id$ */


#ifndef PARSER_XML_H
# define PARSER_XML_H


/* XML */
/* public */
/* types */
typedef struct _XML XML;

typedef struct _XMLAttribute
{
	char * name;
	char * value;
} XMLAttribute;

typedef union _XMLNode XMLNode;

typedef enum _XMLNodeType
{
	XML_NODE_TYPE_TAG,
	XML_NODE_TYPE_DATA
} XMLNodeType;

typedef struct _XMLNodeTag
{
	XMLNodeType type;
	XMLNode * parent;
	char * name;
	XMLAttribute * attributes;
	size_t attributes_cnt;
	XMLNode * nodes;
	size_t nodes_cnt;
} XMLNodeTag;

typedef struct _XMLNodeData
{
	XMLNodeType type;
	XMLNode * parent;
	char * data;
} XMLNodeData;

union _XMLNode
{
	XMLNodeType type;
	struct
	{
		XMLNodeType type;
		XMLNode * parent;
	} parent;
	XMLNodeTag tag;
	XMLNodeData data;
};

typedef struct _XMLDocument
{
	XMLNode * root;
} XMLDocument;


/* functions */
XML * xml_new(char const * pathname);
void xml_delete(XML * xml);

/* accessors */
XMLDocument * xml_get_document(XML * xml);

#endif /* !PARSER_XML_H */
