/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file defines the interface for parse trees.
 * 
 */

#ifndef ParseTree_INTERFACE
#define ParseTree_INTERFACE

typedef struct ParseNode        ParseTree_t;
typedef ParseTree_t P__H       *ParseTreeVia_t;

/* Parse Tree Format Codes */
enum ParseTreeFormat {
    PTF_array_subscript = 1,
    PTF_function_call,
    PTF_message_send,
    PTF_postincrement,
    PTF_postdecrement,
    PTF_argument_list,
    PTF_preincrement,
    PTF_predecrement,
    PTF_address_of,
    PTF_gen_addrof,
    PTF_deref,
    PTF_unary_plus,
    PTF_unary_minus,
    PTF_bitwise_neg,
    PTF_logical_neg,
    PTF_sizeof,
    PTF_enumconstant,
    PTF_multiply,
    PTF_divide,
    PTF_modulo,
    PTF_typechange,
    PTF_add,
    PTF_subtract,
    PTF_shift_left,
    PTF_shift_right,
    PTF_lessthan,
    PTF_greaterthan,
    PTF_lessthaneq,
    PTF_greaterthaneq,
    PTF_equal,
    PTF_notequal,
    PTF_bitwise_and,
    PTF_bitwise_xor,
    PTF_bitwise_or,
    PTF_logical_and,
    PTF_logical_or,
    PTF_ternary,
    PTF_assign,
    PTF_mulassign,
    PTF_divassign,
    PTF_modassign,
    PTF_addassign,
    PTF_subassign,
    PTF_leftassign,
    PTF_rightassign,
    PTF_andassign,
    PTF_xorassign,
    PTF_orassign,
    PTF_commas,
    PTF_multi_initializer,
    PTF_initializer_list,
    PTF_exprstmt,
    PTF_emptystmt,
    PTF_switchcase_stmt,
    PTF_switchdefault_stmt,
    PTF_compound_stmt,
    PTF_stmt_list,
    PTF_ifthenelse_stmt,
    PTF_switch_stmt,
    PTF_while_stmt,
    PTF_dowhile_stmt,
    PTF_for_stmt,
    PTF_continue_stmt,
    PTF_break_stmt,
    PTF_return_stmt,
    PTF_identifier,
    PTF_intconstant,
    PTF_floatconstant,
    PTF_string_literal,
    PTF_struct_member,
    PTF_struct_indirect_member,
    PTF_labelled_stmt,
    PTF_goto_stmt,
    PTF_NOP,
    PTF_asm_stmt
};

#include "TypeRecord.h"
#include "SymTable.h"
#include "FloatLit.h"
#include "CodeGen.h"
#include "Scopes.h"
#include "AbsString.h"


/* ------------------ Parse Trees ------------------ */

/*
 * Below, the structure declaration for Parse Tree nodes.  This structure is
 * used to construct expression trees and statement trees. Each node has a
 * code associated with it, which specifies the meaning of the various
 * fields. The 'kind' field of the nodes in the parse tree indicates the
 * contents of the node.  It may take on any of the values usually used for
 * tokens, in which case it indicates that the node is a leaf of the parse
 * tree, and a terminal symbol.  Or, it may take on a value indicating that
 * it is the parent of several other nodes. Each production in the grammar
 * for this parser has a node format associated with it, and a token code
 * associated with that format. The 'TP' field is used for assigning the
 * types of expressions.
 */

struct ParseNode {
    enum ParseTreeFormat            kind;
    short                           LValue;
    ParseTreeVia_t                  a;
    ParseTreeVia_t                  b;
    ParseTreeVia_t                  c;
    union {
	ScopesVia_t                     Scopes;
	ParseTreeVia_t                  d;
	TypeRecordVia_t                 TP;
    }                               for_Scopes;
    union TokenValue {
	InstListVia_t                   AsmCodes;	/* TODO needs disposal */
	SYMVia_t                        SLit;	/* don't dispose, an alias in
						 * stringlits */
	FloatLitVia_t                   FLit;	/* same as slit */
	long                            number;
	TypeRecordVia_t                 TP;
	SYMVia_t                        thesymbol;	/* don't dispose, in
							 * some symbol table */
	AbsStringID                     identifier;
    }                               data;
};

ParseTreeVia_t
GetTreeFour(ParseTreeVia_t tree);

ScopesVia_t
GetTreeScopes(ParseTreeVia_t tree);

void
                                SetTreeFour(ParseTreeVia_t tree, ParseTreeVia_t d);

void
                                SetTreeScopes(ParseTreeVia_t tree, ScopesVia_t s);

void
                                SetTreeTP(ParseTreeVia_t tree, TypeRecordVia_t TP);

TypeRecordVia_t
GetTreeTP(ParseTreeVia_t tree);

short
                                GetTreeLValue(ParseTreeVia_t tree);

void
                                SetTreeLValue(ParseTreeVia_t tree, short v);

#endif
