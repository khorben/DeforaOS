/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel c99 */
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



#include "common.h"
#include "tokenset.h"


/* protected */
/* variables */
/* abstract_declarator */
static TokenCode _c99set_abstract_declarator[] =
{
#include "sets/abstract_declarator.set"
	C99_CODE_NULL
};

TokenSet c99set_abstract_declarator = _c99set_abstract_declarator;


/* abstract_or_declarator */
static TokenCode _c99set_abstract_or_declarator[] =
{
#include "sets/abstract_or_declarator.set"
	C99_CODE_NULL
};

TokenSet c99set_abstract_or_declarator = _c99set_abstract_or_declarator;


/* argument-expr-list */
static TokenCode _c99set_argument_expr_list[] =
{
#include "sets/argument_expr_list.set"
	C99_CODE_NULL
};

TokenSet c99set_argument_expr_list = _c99set_argument_expr_list;


/* assignment-expr */
static TokenCode _c99set_assignment_expr[] =
{
#include "sets/assignment_expr.set"
	C99_CODE_NULL
};

TokenSet c99set_assignment_expr = _c99set_assignment_expr;


/* assignment-operator */
static TokenCode _c99set_assignment_operator[] =
{
#include "sets/assignment_operator.set"
	C99_CODE_NULL
};

TokenSet c99set_assignment_operator = _c99set_assignment_operator;


/* block-item */
static TokenCode _c99set_block_item[] =
{
#include "sets/block_item.set"
	C99_CODE_NULL
};

TokenSet c99set_block_item = _c99set_block_item;


/* block-item-list */
static TokenCode _c99set_block_item_list[] =
{
#include "sets/block_item_list.set"
	C99_CODE_NULL
};

TokenSet c99set_block_item_list = _c99set_block_item_list;


/* compound-statement */
static TokenCode _c99set_compound_statement[] =
{
#include "sets/compound_statement.set"
	C99_CODE_NULL
};

TokenSet c99set_compound_statement = _c99set_compound_statement;


/* declaration */
static TokenCode _c99set_declaration[] =
{
#include "sets/declaration.set"
	C99_CODE_NULL
};

TokenSet c99set_declaration = _c99set_declaration;


/* declaration-list */
static TokenCode _c99set_declaration_list[] =
{
#include "sets/declaration_list.set"
	C99_CODE_NULL
};

TokenSet c99set_declaration_list = _c99set_declaration_list;


/* declaration-specifiers */
static TokenCode _c99set_declaration_specifiers[] =
{
#include "sets/declaration_specifiers.set"
	C99_CODE_NULL
};

TokenSet c99set_declaration_specifiers = _c99set_declaration_specifiers;


/* declarator */
static TokenCode _c99set_declarator[] =
{
#include "sets/declarator.set"
	C99_CODE_NULL
};

TokenSet c99set_declarator = _c99set_declarator;


/* designation */
static TokenCode _c99set_designation[] =
{
#include "sets/designation.set"
	C99_CODE_NULL
};

TokenSet c99set_designation = _c99set_designation;


/* designator */
static TokenCode _c99set_designator[] =
{
#include "sets/designator.set"
	C99_CODE_NULL
};

TokenSet c99set_designator = _c99set_designator;


/* direct-abstract-declarator */
static TokenCode _c99set_direct_abstract_declarator[] =
{
#include "sets/direct_abstract_declarator.set"
	C99_CODE_NULL
};

TokenSet c99set_direct_abstract_declarator = _c99set_direct_abstract_declarator;


/* enum-specifier */
static TokenCode _c99set_enum_specifier[] =
{
#include "sets/enum_specifier.set"
	C99_CODE_NULL
};

TokenSet c99set_enum_specifier = _c99set_enum_specifier;


/* enumerator */
static TokenCode _c99set_enumerator[] =
{
#include "sets/enumerator.set"
	C99_CODE_NULL
};

TokenSet c99set_enumerator = _c99set_enumerator;


/* expression */
static TokenCode _c99set_expression[] =
{
#include "sets/expression.set"
	C99_CODE_NULL
};

TokenSet c99set_expression = _c99set_expression;


/* expression-statement */
static TokenCode _c99set_expression_statement[] =
{
#include "sets/expression_statement.set"
	C99_CODE_NULL
};

TokenSet c99set_expression_statement = _c99set_expression_statement;


/* external-declaration */
static TokenCode _c99set_external_declaration[] =
{
#include "sets/external_declaration.set"
	C99_CODE_NULL
};

TokenSet c99set_external_declaration = _c99set_external_declaration;


/* function-definition */
static TokenCode _c99set_function_definition[] =
{
#include "sets/function_definition.set"
	C99_CODE_NULL
};

TokenSet c99set_function_definition = _c99set_function_definition;


/* function-specifier */
static TokenCode _c99set_function_specifier[] =
{
#include "sets/function_specifier.set"
	C99_CODE_NULL
};

TokenSet c99set_function_specifier = _c99set_function_specifier;


/* identifier */
static TokenCode _c99set_identifier[] =
{
#include "sets/identifier.set"
	C99_CODE_NULL
};

TokenSet c99set_identifier = _c99set_identifier;


/* identifier-list */
static TokenCode _c99set_identifier_list[] =
{
#include "sets/identifier_list.set"
	C99_CODE_NULL
};

TokenSet c99set_identifier_list = _c99set_identifier_list;


/* init-declarator-list */
static TokenCode _c99set_init_declarator_list[] =
{
#include "sets/init_declarator_list.set"
	C99_CODE_NULL
};

TokenSet c99set_init_declarator_list = _c99set_init_declarator_list;


/* iteration-statement */
static TokenCode _c99set_iteration_statement[] =
{
#include "sets/iteration_statement.set"
	C99_CODE_NULL
};

TokenSet c99set_iteration_statement = _c99set_iteration_statement;


/* jump-statement */
static TokenCode _c99set_jump_statement[] =
{
#include "sets/jump_statement.set"
	C99_CODE_NULL
};

TokenSet c99set_jump_statement = _c99set_jump_statement;


/* keyword */
static TokenCode _c99set_keyword[] =
{
#include "sets/keyword.set"
	C99_CODE_NULL
};

TokenSet c99set_keyword = _c99set_keyword;


/* labeled-statement */
static TokenCode _c99set_labeled_statement[] =
{
#include "sets/labeled_statement.set"
	C99_CODE_NULL
};

TokenSet c99set_labeled_statement = _c99set_labeled_statement;


/* parameter-type-list */
static TokenCode _c99set_parameter_type_list[] =
{
#include "sets/parameter_type_list.set"
	C99_CODE_NULL
};

TokenSet c99set_parameter_type_list = _c99set_parameter_type_list;


/* pointer */
static TokenCode _c99set_pointer[] =
{
#include "sets/pointer.set"
	C99_CODE_NULL
};

TokenSet c99set_pointer = _c99set_pointer;


/* postfix-expr */
static TokenCode _c99set_postfix_expr[] =
{
#include "sets/postfix_expr.set"
	C99_CODE_NULL
};

TokenSet c99set_postfix_expr = _c99set_postfix_expr;


/* primary-expr */
static TokenCode _c99set_primary_expr[] =
{
#include "sets/primary_expr.set"
	C99_CODE_NULL
};

TokenSet c99set_primary_expr = _c99set_primary_expr;


/* punctuator */
static TokenCode _c99set_punctuator[] =
{
#include "sets/punctuator.set"
	C99_CODE_NULL
};

TokenSet c99set_punctuator = _c99set_punctuator;


/* selection-statement */
static TokenCode _c99set_selection_statement[] =
{
#include "sets/selection_statement.set"
	C99_CODE_NULL
};

TokenSet c99set_selection_statement = _c99set_selection_statement;


/* statement */
static TokenCode _c99set_statement[] =
{
#include "sets/statement.set"
	C99_CODE_NULL
};

TokenSet c99set_statement = _c99set_statement;


/* storage-class-specifier */
static TokenCode _c99set_storage_class_specifier[] =
{
#include "sets/storage_class_specifier.set"
	C99_CODE_NULL
};

TokenSet c99set_storage_class_specifier = _c99set_storage_class_specifier;


/* struct-declaration */
static TokenCode _c99set_struct_declaration[] =
{
#include "sets/struct_declaration.set"
	C99_CODE_NULL
};

TokenSet c99set_struct_declaration = _c99set_struct_declaration;


/* struct-declaration-list */
static TokenCode _c99set_struct_declaration_list[] =
{
#include "sets/struct_declaration_list.set"
	C99_CODE_NULL
};

TokenSet c99set_struct_declaration_list = _c99set_struct_declaration_list;


/* struct-or-union-specifier */
static TokenCode _c99set_struct_or_union_specifier[] =
{
#include "sets/struct_or_union_specifier.set"
	C99_CODE_NULL
};

TokenSet c99set_struct_or_union_specifier = _c99set_struct_or_union_specifier;


/* type-qualifier */
static TokenCode _c99set_type_qualifier[] =
{
#include "sets/type_qualifier.set"
	C99_CODE_NULL
};

TokenSet c99set_type_qualifier = _c99set_type_qualifier;


/* type-qualifier-list */
TokenSet c99set_type_qualifier_list = _c99set_type_qualifier;


/* type-specifier */
static TokenCode _c99set_type_specifier[] =
{
#include "sets/type_specifier.set"
	C99_CODE_NULL
};

TokenSet c99set_type_specifier = _c99set_type_specifier;


/* type-name */
static TokenCode _c99set_type_name[] =
{
#include "sets/type_name.set"
	C99_CODE_NULL
};

TokenSet c99set_type_name = _c99set_type_name;


/* typedef-name */
static TokenCode _c99set_typedef_name[] =
{
#include "sets/typedef_name.set"
	C99_CODE_NULL
};

TokenSet c99set_typedef_name = _c99set_typedef_name;


/* unary-expr */
static TokenCode _c99set_unary_expr[] =
{
#include "sets/unary_expr.set"
	C99_CODE_NULL
};

TokenSet c99set_unary_expr = _c99set_unary_expr;


/* unary-operator */
static TokenCode _c99set_unary_operator[] =
{
#include "sets/unary_operator.set"
	C99_CODE_NULL
};

TokenSet c99set_unary_operator = _c99set_unary_operator;
