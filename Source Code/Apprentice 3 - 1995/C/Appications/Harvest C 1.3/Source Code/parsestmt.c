/*
	Harvest C
	Copyright 1992 Eric W. Sink.  All rights reserved.
	
	This file is part of Harvest C.
	
	Harvest C is free software; you can redistribute it and/or modify
	it under the terms of the GNU Generic Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.
	
	Harvest C is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with Harvest C; see the file COPYING.  If not, write to
	the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
	
	Harvest C is not in any way a product of the Free Software Foundation.
	Harvest C is not GNU software.
	Harvest C is not public domain.

	This file may have other copyrights which are applicable as well.

*/

/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file contains the parser for statements.
 * 
 */


#include "conditcomp.h"
#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "as.h"
#include "lookup.h"
#include "regs.h"

#pragma segment ParseStmt


/* THEPARSER */

/*
 * Parsing:  We conceptually separate parsing into 2 stages : syntactic
 * parsing and semantic parsing.  During syntactic parsing, we wish to
 * identify all problems occuring in syntax only - such as unmatched parens.
 * During semantic parsing, we wish to identify all other problems which will
 * prohibit us from producing assembler output.  These problems include : use
 * of an undefined identifier, type mismatches, etc...
 */
/*
 * This compiler does not implement volatile.
 */

/*
 * Every routine which returns an expression, must set the type of that
 * expression and whether or not it is an lvalue.  In fact, any such routine
 * should have the following four items, for every call to BuildTree*().  1.
 * The BuildTree*() call itself, 2.  Explicit setting of the type, 3.
 * Explicit setting of the LValue status, and 4.  a TYPERULE comment,
 * explaining what the rules behind that set were.  TYPERULE comments may
 * also explain limitations on operands and the like.
 */

#ifdef INLINEASM

LocAMVia_t
Do_asm_operand(void)
{				/* TODO This routine not done */
    LocAMVia_t                      result = NULL;
    if (NextIs('#')) {
	if (NextIs(INTCONSTANT)) {
	    result = BuildImmediate(LastIntegerConstant, M68sz_long);
	} else {
	    SyntaxError("Immediate what ?!?");
	}
    } else if (NextIs('{')) {
	/* FIELD, DYNK, STATK */
    } else if (NextIs('(')) {
	/* Could be almost anything */
	if (NextIs('[')) {
	} else {
	    if (GetIDENTIFIER()) {
		/* Need more cases here... */
	    }
	}
    } else if (NextIs('-')) {
	if (NextIs('(')) {
	    if (GetIDENTIFIER()) {
		result = BuildARegPreDec(LastToken[1] - '0');
		if (!NextIs(')'))
		    SyntaxError("Bad asm operand");
	    } else {
		SyntaxError("Bad asm operand");
	    }
	} else {
	    SyntaxError("Bad asm operand");
	}
    } else if (GetIDENTIFIER()) {
	/* We first lookup to see if it's a register name */
	int                             ndx = 0;
	int                             found = 0;
	while (!found && (iregs[ndx].r_name)) {
	    if (!strcmp(LastToken, iregs[ndx].r_name)) {
		found = ndx;
	    }
	    ndx++;
	}
	if (found) {
	    result = RawLocation();
	    Via(result)->AM = M68am_OtherFormat;
	    Via(result)->OtherFormat = Ealloc(sizeof(struct ea));
	    Via(Via(result)->OtherFormat)->type = iregs[found].r_type;
	    Via(Via(result)->OtherFormat)->reg = iregs[found].r_value;
	} else {
	    /* Lookup to see if it's a C identifier */
	    SYMVia_t                        symbol;
	    symbol = LookUpSymbol(LastToken);
	    if (symbol) {
		result = RawLocation();
		Via(result)->AM = M68am_CIdentifier;
		/* TODO */
	    }
	}
    } else if (NextIs(INTCONSTANT)) {
	/* Indexed ?? */
	/* TODO Indexed parsing */
	long                            offset = LastIntegerConstant;
	if (NextIs('(')) {
	    /* Info from do_indexed() */
	    result = RawLocation();
	    Via(result)->Constant = offset;
	}
    }
    return result;
}

ParseTreeVia_t
Do_assembler_statement(void)
{
    /* asm { list of instructions } */
    /* Each instruction must be terminated in a semicolon */
    ParseTreeVia_t                  result;
    struct mne                     *mneop;
    enum Size68                     SZ = M68sz_none;
    LocAMVia_t                      loc1 = NULL;
    LocAMVia_t                      loc2 = NULL;
    LocAMVia_t                      loc3 = NULL;
    int                             done = 0;
    result = NULL;
    if (NextIs(ASM)) {
	if (!NextIs('{'))
	    SyntaxError("Asm must be followed by a bracket");
	result = BuildTreeNode(PTF_asm_stmt, NULL, NULL, NULL);
	Via(result)->data.AsmCodes = RawInstructionList();
	while (!done) {
	    LexerGetLine(asmLine);
	    if (parse_line()) {
		if (!process_inlineasm(Via(result)->data.AsmCodes))
		    done = 1;
	    } else
		done = 1;
	}
	PushSRC();
	CurrentSRC.isIO = 0;
	CurrentSRC.where.mem = asmLine;
	if (!NextIs('}'))
	    SyntaxError("Asm must be followed by a bracket");
    }
    return result;
}

#endif

ParseTreeVia_t
Do_statement(void)
{
    /*
     * statement : expression_statement | labeled_statement |
     * compound_statement | assembler_statement | selection_statement |
     * iteration_statement | jump_statement ;
     */

    /*
     * No nodes need to be built here.  Each of the types of statements,
     * identifies itself adequately.
     */
    ParseTreeVia_t                  result;
    int                             StartLine;
    StartLine = CurrentSRC.LineCount;
    if (result = Do_labeled_statement()) {
    } else if (result = Do_expression_statement()) {
    } else if (result = Do_compound_statement(NULL)) {
#ifdef INLINEASM
    } else if (result = Do_assembler_statement()) {
#endif
    } else if (result = Do_selection_statement()) {
    } else if (result = Do_iteration_statement()) {
    } else if (result = Do_jump_statement()) {
    } else {
	result = NULL;
    }
    if (result) {
	CurrentSRC.StmtCount++;
#ifdef Undefined
	/* This was removed 28 April 1992.  It has been the cause of a number
	of bugs, and I don't think it provides all that much benefit when
	it's working. */
	result = Constify(result);
#endif
    }
    return result;
}

ParseTreeVia_t
Do_labeled_statement(void)
{
    /*
     * labeled_statement : IDENTIFIER ':' statement | CASE constant_expr ':'
     * statement | DEFAULT ':' statement ;
     */

    ParseTreeVia_t                  result = NULL;
    if (NextIs(IDENTIFIER)) {
	char                            theid[128];
	strcpy(theid, LastToken);
	if (NextIs(':')) {
	    result = Do_statement();
	    if (result) {
		SYMVia_t                        label;
		result = BuildIDTreeNode(PTF_labelled_stmt, result, NULL, NULL, theid);
		label = TableSearch(CurrentLabels(), (theid));
		if (!label) {
		    label = TableAdd(CurrentLabels(), (theid));
		}
		Via(label)->Definition.Stmt = result;
	    } else {
		SyntaxError("Expected statement following label");
	    }
	} else {
	    /*
	     * This is not an error, because the identifier is probably the
	     * beginning of an expression.  Labelled statements are parsed
	     * before expression statements for this reason.
	     */
	    PutBackToken(theid, IDENTIFIER);
	    result = NULL;
	}
    } else if (NextIs(CASE)) {
	ParseTreeVia_t                  tmp2;
	FunctionComplexity++;
	if (!LastSwitchExpression) {
	    SyntaxError("case found outside of switch statement");
	}
	tmp2 = Do_constant_expr();
	if (tmp2) {
	    /*
	     * TODO Compare type here with that of the expr in
	     * LastSwitchExpression.
	     */
	    if (NextIs(':')) {
		result = Do_statement();
		if (result) {
		    result = BuildTreeNode(PTF_switchcase_stmt, tmp2, result, NULL);
		} else {
		    SyntaxError("Expected statement following case label");
		}
	    } else {
		SyntaxError("Expected ':' after case constant");
	    }
	} else {
	    SyntaxError("Expected constant expr for case label");
	}
    } else if (NextIs(DEFAULT)) {
	FunctionComplexity++;
	if (!LastSwitchExpression) {
	    SyntaxError("default found outside of switch statement");
	}
	if (NextIs(':')) {
	    result = Do_statement();
	    if (result) {
		result = BuildTreeNode(PTF_switchdefault_stmt, result, NULL, NULL);
	    } else {
		SyntaxError("Expected statement following default label");
	    }
	} else {
	    SyntaxError("default must be followed by a colon");
	}
    } else {
	result = NULL;
    }
    return result;
}

ParseTreeVia_t
Do_compound_statement(SYMVia_t functop)
{
    /*
     * compound_statement : '{' '}' | '{' statement_list '}' | '{'
     * declaration_list '}' | '{' declaration_list statement_list '}' ;
     */

    ParseTreeVia_t                  result;
    int                             declresult;
    ScopesVia_t                     thescopes;
    if (NextIs('{')) {
	ParseTreeVia_t                  tmp;
	ExtraBlocks = 0;
	if (functop) {
	    if (GetTPKind(Via(functop)->TP) == TRC_typedef) {
		DeclError("A function cannot gain that status by means of typedef");
	    }
	}
	result = BuildTreeNode(PTF_compound_stmt, NULL, NULL, NULL);
	SetTreeScopes(result, thescopes = Ealloc(sizeof(Scopes_t)));
	Via(thescopes)->Symbols = RawTable(11);
	Via(thescopes)->Tags = RawTable(11);
	Via(thescopes)->Enums = NULL;
	if (functop) {
	    Via(thescopes)->Labels = RawTable(11);
	    PushSpaces(GetTPMembers(Via(functop)->TP), NULL, NULL);	/* function parameters */
	} else {
	    Via(thescopes)->Labels = NULL;	/* We don't push a table
						 * here, thus keeping the
						 * previous scope level for
						 * labels, because the scope
						 * of a label is the entire
						 * function in which it
						 * resides.  The label table
						 * for each function is
						 * pushed only once. */
	}
	PushSpaces(Via(thescopes)->Symbols,
		   Via(thescopes)->Tags,
		   Via(thescopes)->Labels);
	if (NextIs('}')) {
	    if (ExtraBlocks) {
		    while (ExtraBlocks--)
			PopBlock();
	    }
	    PopBlock();
	    if (functop) {
		PopBlock();
	    }
	    UserWarning(WARN_emptycompound);
	} else {
	    char                            nm[64];
	    declresult = Do_declaration_list(Via(thescopes)->Symbols);
	    if (functop) {
		/*
		 * Check all members of the just-parsed declaration list, to
		 * make sure there are no conflicts with declared args
		 */
		int                             ndx;
		SYMVia_t                        cur;
		ndx = Via(Via(thescopes)->Symbols)->count;
		while (ndx) {
		    cur = TableGetNum(Via(thescopes)->Symbols, ndx);
		    GetSymName(cur, nm);
		    if (TableSearch(GetTPMembers(Via(functop)->TP), nm)) {
			DeclError("Local variable conflicts with parameter");
		    }
		    ndx--;
		}
	    }
	    tmp = Do_statement_list();
	    CheckUsages(Via(thescopes)->Symbols);
	    if (ExtraBlocks) {
		    while (ExtraBlocks--)
		        PopBlock();
	    }
	    PopBlock();
	    if (functop) {
		PopBlock();
	    }
	    if (declresult) {
		if (tmp) {
		    Via(result)->a = tmp;
		    if (!NextIs('}')) {
			SyntaxError("Missing right brace");
		    }
		} else {
		    if (!NextIs('}')) {
			SyntaxError("Missing right brace");
		    }
		}
	    } else if (tmp) {
		Via(result)->a = tmp;
		if (!NextIs('}')) {
		    SyntaxError("Missing right brace");
		}
	    } else {
		SyntaxError("Missing right brace");
	    }
	}
    } else {
	result = NULL;
    }
    return result;
}

int
isJumpStmt(ParseTreeVia_t tree)
{
    if (!tree) {
	return 0;
    }
    switch (Via(tree)->kind) {
    case PTF_goto_stmt:
    case PTF_break_stmt:
    case PTF_continue_stmt:
    case PTF_return_stmt:
	return 1;
	break;
    default:
	return 0;
	break;
    }
}

int
isLabeledStmt(ParseTreeVia_t tree)
{
    if (!tree) {
	return 0;
    }
    switch (Via(tree)->kind) {
    case PTF_labelled_stmt:
    case PTF_switchcase_stmt:
    case PTF_switchdefault_stmt:
	return 1;
	break;
    default:
	return 0;
	break;
    }
}

ParseTreeVia_t
Do_statement_list(void)
{
    /*
     * statement_list : statement | statement_list statement ;
     */
    ParseTreeVia_t                  result;
    int                             donelisting;
    int                             lastwasjump;
    result = Do_statement();
    if (result) {
	lastwasjump = isJumpStmt(result);
	donelisting = 0;
	while (!donelisting) {
	    ParseTreeVia_t                  tmp;
	    tmp = Do_statement();
	    if (tmp) {
		if (lastwasjump && isLabeledStmt(tmp)) {
		    lastwasjump = 0;
		}
		if (lastwasjump && !isLabeledStmt(tmp)) {
		    UserWarning(WARN_deadcode);
		    /* TODO We should consider removal of the dead code. */
		}
		result = BuildTreeNode(PTF_stmt_list, tmp, result, NULL);
		/*
		 * Here is an example of a parse tree list. We want to be
		 * sure that whatever gets returned from this routine has a
		 * valid statement in a, and an optional tail of list in b.
		 * In other words, when constructing a list, the tail should
		 * always be in b, not a.
		 */
	    } else {
		donelisting = 1;
	    }
	}
    }
    return result;
}

/*----------------------------------------*/

ParseTreeVia_t
Do_expression_statement(void)
{
    /*
     * expression_statement : ';' | expr ';' ;
     */

    ParseTreeVia_t                  result;
    result = NULL;
    if (NextIs(';')) {
	result = BuildTreeNode(PTF_emptystmt, NULL, NULL, NULL);
	UserWarning(WARN_emptystatement);
    } else {
	result = Do_expr();
	if (result) {
	    if (Via(result)->kind == PTF_function_call) {
		if (!isVoidType(GetTreeTP(result))) {
		    UserWarning(WARN_discardfuncresult);
		}
	    }
	    result = BuildTreeNode(PTF_exprstmt, result, NULL, NULL);
	    if (!NextIs(';')) {
		SyntaxError("Missing semicolon");
	    }
	}
    }
    return result;
}

ParseTreeVia_t
Do_selection_statement(void)
{
    /*
     * selection_statement : IF '(' expr ')' statement | IF '(' expr ')'
     * statement ELSE statement | SWITCH '(' expr ')' statement ;
     */

    ParseTreeVia_t                  result = NULL;
    ParseTreeVia_t                  tmp;
    FoldValue_t                     testK;
    if (NextIs(IF)) {
	FunctionComplexity++;
	if (NextIs('(')) {
	    tmp = Do_expr();
	    if (tmp) {
		if (Via(tmp)->kind == PTF_assign) {
		    UserWarning(WARN_assignif);
		}
		ConstExprValue(tmp, &testK);
		if (testK.isK) {
		    UserWarning(WARN_constantif);
		}
		if (!isBooleanType(GetTreeTP(tmp))) {
		    TypeError("if expression must be arithmetic or pointer type");
		}
		if (NextIs(')')) {
		    result = Do_statement();
		    if (result) {
			if (NextIs(ELSE)) {
			    ParseTreeVia_t                  tmp2;
			    FunctionComplexity++;
			    tmp2 = Do_statement();
			    if (tmp2) {
				result = BuildTreeNode(PTF_ifthenelse_stmt, tmp, result, tmp2);
			    } else {
				SyntaxError("Expected statement following else");
			    }
			} else {
			    result = BuildTreeNode(PTF_ifthenelse_stmt, tmp, result, NULL);
			}
		    } else {
			SyntaxError("Expected statement following if");
		    }
		} else {
		    SyntaxError("Missing right parenthesis");
		}
	    } else {
		SyntaxError("Expected expr for if conditional");
	    }
	} else {
	    SyntaxError("if keyword must be followed by a left paren");
	}
    } else if (NextIs(SWITCH)) {
	ParseTreeVia_t                  oldLastSwitch = NULL;
	if (NextIs('(')) {
	    tmp = Do_expr();
	    if (tmp) {
		ConstExprValue(tmp, &testK);
		if (testK.isK) {
		    UserWarning(WARN_constantswitch);
		}
		if (!isIntegralType(GetTreeTP(tmp))) {
		    UserWarning(WARN_nonintegralswitch);
		}
		oldLastSwitch = LastSwitchExpression;
		LastSwitchExpression = tmp;
		if (NextIs(')')) {
		    result = Do_statement();
		    if (result) {
			result = BuildTreeNode(PTF_switch_stmt, tmp, result, NULL);
		    } else {
			SyntaxError("Expected statement after switch (cond)");
		    }
		} else {
		    SyntaxError("Missing right parenthesis");
		}
	    } else {
		SyntaxError("Expected expression for switch");
	    }
	} else {
	    SyntaxError("switch must be followed by a paren");
	}
	LastSwitchExpression = oldLastSwitch;
    } else {
	result = NULL;
    }
    return result;
}

ParseTreeVia_t
Do_iteration_statement(void)
{
    /*
     * iteration_statement : WHILE '(' expr ')' statement | DO statement
     * WHILE '(' expr ')' ';' | FOR '(' ';' ';' ')' statement | FOR '(' ';'
     * ';' expr ')' statement | FOR '(' ';' expr ';' ')' statement | FOR '('
     * ';' expr ';' expr ')' statement | FOR '(' expr ';' ';' ')' statement |
     * FOR '(' expr ';' ';' expr ')' statement | FOR '(' expr ';' expr ';'
     * ')' statement | FOR '(' expr ';' expr ';' expr ')' statement ;
     */

    ParseTreeVia_t                  result = NULL;
    ParseTreeVia_t                  tmp1;
    ParseTreeVia_t                  tmp2;
    ParseTreeVia_t                  tmp3;
    ParseTreeVia_t                  other;
    FoldValue_t                     testK;
    if (NextIs(WHILE)) {
	FunctionComplexity++;
	if (NextIs('(')) {
	    tmp1 = Do_expr();
	    if (tmp1) {
		if (!isBooleanType(GetTreeTP(tmp1))) {
		    TypeError("while expression must be arithmetic or pointer type");
		}
		ConstExprValue(tmp1, &testK);
		if (testK.isK) {
		    UserWarning(WARN_constantwhile);
		}
		if (NextIs(')')) {
		    tmp2 = Do_statement();
		    if (tmp2) {
			result = BuildTreeNode(PTF_while_stmt, tmp1, tmp2, NULL);
		    } else {
			SyntaxError("Expected statement for while loop");
		    }
		} else {
		    SyntaxError("Missing right parenthesis");
		}
	    } else {
		SyntaxError("Expected expression for while loop");
	    }
	} else {
	    SyntaxError("while must be followed by a left parenthesis");
	}
    } else if (NextIs(DO)) {
	tmp1 = Do_statement();
	if (tmp1) {
	    if (NextIs(WHILE)) {
		FunctionComplexity++;
		if (NextIs('(')) {
		    tmp2 = Do_expr();
		    if (tmp2) {
			ConstExprValue(tmp2, &testK);
			if (testK.isK) {
			    UserWarning(WARN_constantdowhile);
			}
			if (!isBooleanType(GetTreeTP(tmp2))) {
			    TypeError("while expression must be arith or pointer type");
			}
			result = BuildTreeNode(PTF_dowhile_stmt, tmp1, tmp2, NULL);
			if (!NextIs(')')) {
			    SyntaxError("Missing right parenthesis");
			}
			if (!NextIs(';')) {
			    SyntaxError("Missing semicolon");
			}
		    } else {
			SyntaxError("Expected expression for do-while");
		    }
		} else {
		    SyntaxError("Expected left parenthesis");
		}
	    } else {
		SyntaxError("Expected while keyword after do statement");
	    }
	}
    } else if (NextIs(FOR)) {
	FunctionComplexity++;
	tmp1 = tmp2 = tmp3 = other = NULL;
	if (NextIs('(')) {
	    tmp1 = Do_expr();
	    if (!NextIs(';')) {
		SyntaxError("Missing semicolon");
	    }
	    tmp2 = Do_expr();
	    if (tmp2) {
		ConstExprValue(tmp2, &testK);
		if (testK.isK) {
		    UserWarning(WARN_constantfor);
		}
		if (!isBooleanType(GetTreeTP(tmp2))) {
		    TypeError("middle for expression must be arithmetic or pointer type");
		}
	    }
	    if (!NextIs(';')) {
		SyntaxError("Missing semicolon");
	    }
	    tmp3 = Do_expr();
	    if (!NextIs(')')) {
		SyntaxError("Missing right parenthesis");
	    }
	    other = Do_statement();
	    if (other) {
		result = BuildTreeNode(PTF_for_stmt, tmp1, tmp2, tmp3);
		SetTreeFour(result, other);
	    } else {
		SyntaxError("Expected statement for for loop");
	    }
	} else {
	    SyntaxError("for must be followed by a left parenthesis");
	}
    } else {
	result = NULL;
    }
    return result;
}

ParseTreeVia_t
Do_jump_statement(void)
{
    /*
     * jump_statement : GOTO IDENTIFIER ';' | CONTINUE ';' | BREAK ';' |
     * RETURN ';' | RETURN expr ';' ;
     */

    ParseTreeVia_t                  result;
    if (NextIs(GOTO)) {
	UserWarning(WARN_goto);
	if (NextIs(IDENTIFIER)) {
	    SYMVia_t                        label;
	    label = TableSearch(CurrentLabels(), (LastToken));
	    if (!label) {
		label = TableAdd(CurrentLabels(), (LastToken));
	    }
	    result = BuildIDTreeNode(PTF_goto_stmt, NULL, NULL, NULL, LastToken);
	    if (!NextIs(';')) {
		SyntaxError("Missing semicolon");
	    }
	} else {
	    SyntaxError("Expected label after goto");
	}
    } else if (NextIs(CONTINUE)) {
	if (NextIs(';')) {
	    result = BuildTreeNode(PTF_continue_stmt, NULL, NULL, NULL);
	} else {
	    SyntaxError("Missing semicolon");
	}
    } else if (NextIs(BREAK)) {
	if (NextIs(';')) {
	    result = BuildTreeNode(PTF_break_stmt, NULL, NULL, NULL);
	} else {
	    SyntaxError("Missing semicolon");
	}
    } else if (NextIs(RETURN)) {
	FunctionReturnCount++;
	if (NextIs(';')) {
	    TypeRecordVia_t                 returntypeSB;
	    returntypeSB = isFunctionType(GetSymTP(FunctionBeingDefined));
	    if (!isVoidType(returntypeSB)) {
		TypeError("return void in non-void valued function");
	    }
	    result = BuildTreeNode(PTF_return_stmt, NULL, NULL, NULL);
	} else {
	    if ((result = Do_expr()) != 0) {
		TypeRecordVia_t                 returntypeSB;
		returntypeSB = isFunctionType(GetSymTP(FunctionBeingDefined));
		ReturnCoerce(returntypeSB, &result);
		if (!SameType(returntypeSB, GetTreeTP(result))) {
		    TypeError("type of return expression does not match function type");
		}
		result = BuildTreeNode(PTF_return_stmt, result, NULL, NULL);
		if (!NextIs(';')) {
		    SyntaxError("Missing semicolon");
		}
	    } else {
		SyntaxError("Missing semicolon");
	    }
	}
    } else {
	result = NULL;
    }
    return result;
}

ParseTreeVia_t
Do_function_body(SYMVia_t funcname)
{
    /*
     * function_body : compound_statement | declaration_list
     * compound_statement ;
     */
    /*
     * There IS a difference between old style and new style function
     * declarations.  With new style declarations, when the function is
     * called, the arguments are type checked against the parameters. With
     * old style, only the return type of the function is stored in the
     * symbol table, and arguments cannot be type-checked.  In this case,
     * standard rules specify how the arguments are to be promoted before
     * pushing them on the stack.
     */
    /*
     * Within the body of a function, there are three symbol tables for
     * objects in scope.  The first is the local symbol table for the block.
     * (In the case of nested blocks, there may be more than three symbol
     * tables in scope)  The second is the table of arguments to the function
     * (this is stored in the symbol table record for the name of the
     * function.)  The third is the global symbol table.  They should be
     * checked in the order given here. That checking is done in
     * do_postfix_expr().
     */
    ParseTreeVia_t                  result;
    int                             tmp;
    SymListVia_t                    oldstyles;
    int                             protospushed;
    int                             PrevStmtCount;
    extern LabSYMVia_t NextFuncSegment;
    PrevStmtCount = CurrentSRC.StmtCount;
    protospushed = 0;
    FunctionBeingDefined = funcname;
    NextFuncSegment = CurrentSegmentLabel;
    FunctionReturnCount = 0;
    FunctionComplexity = 1;
    if (!isFunctionType(GetSymTP(funcname))) {
	SyntaxError("Malformed declaration");
	return NULL;
    }
    oldstyles = RawTable(11);
    tmp = Do_declaration_list(oldstyles);
    if (tmp && (GetTPKind(GetSymTP(funcname)) != TRC_OLDfunction)) {
	DeclErrorSYM("Inconsistent function declaration", funcname);
    }
    if (tmp) {
	int                             cnt;
	SYMVia_t                        outp;
	cnt = 1;
	/* Modify array and function types as per ANSI */
	while (cnt <= Via(oldstyles)->count) {
	    outp = TableGetNum(oldstyles, cnt);
	    if (isArrayType(GetSymTP(outp))) {
		SetSymTP(outp, BuildTypeRecord(GetTPBase(GetSymTP(outp)), TRC_pointer, SGN_unknown));
	    }
	    if (isFunctionType(GetSymTP(outp))) {
		SetSymTP(outp, BuildTypeRecord(GetSymTP(outp), TRC_pointer, SGN_unknown));
	    }
	    cnt++;
	}
    }
    if (tmp) {
	int                             cnt;
	SYMVia_t                        inp;
	SYMVia_t                        outp;
	char                            nm[64];
	cnt = 1;
	while (cnt <= Via(oldstyles)->count) {
	    outp = TableGetNum(oldstyles, cnt);
	    GetSymName(outp, nm);
	    inp = TableSearch(GetTPMembers(GetSymTP(funcname)), nm);
	    if (inp) {
		if (GetSymTP(inp)) {
		    if (!SameType(GetSymTP(inp), GetSymTP(outp))) {
			DeclErrorSYM("Inconsistent function declaration", funcname);
		    }
		} else {
		    SetSymTP(inp, GetSymTP(outp));
		}
	    } else {
		DeclError2("Not a parameter : ", nm);
	    }
	    /*
	     * If the members in inp have type associated with them, then
	     * this was previously declared ANSI, and we are merely checking
	     * for consistency.
	     */
	    cnt++;
	}
	if (GetTPKind(GetSymTP(funcname)) != TRC_ANSIfunction) {
		    cnt = 1;
		    while (cnt <= Via(GetTPMembers(GetSymTP(funcname)))->count) {
			inp = TableGetNum(GetTPMembers(GetSymTP(funcname)), cnt);
			if (inp) {
			    if (GetSymTP(inp)) {
			    	/* Don't promote args per ANSI if the function
			    		is a pascal function. 28 March 1992 */
				if (!(GetTPFlags(GetSymTP(funcname)) & ISPASCALMASK)) {
					/* Other possibilities might apply here. */
					if (GetTPKind(GetSymTP(inp)) == TRC_char) {
					    SetSymTP(inp, BuildTypeRecord(0, TRC_int, SGN_signed));
					}
					if (GetTPKind(GetSymTP(inp)) == TRC_short) {
					    SetSymTP(inp, BuildTypeRecord(0, TRC_int, SGN_signed));
					}
				}
			    } else {
				SetSymTP(inp, BuildTypeRecord(0, TRC_int, SGN_signed));
			    }
			}
			cnt++;
		    }
	}
    }
    FreeSymbolList(oldstyles, 0);
    result = Do_compound_statement(funcname);
    if (FunctionReturnCount) {
	if (FunctionReturnCount > 1) {
	    UserWarning(WARN_multireturn);
	}
    } else {
	if (!(isVoidType(isFunctionType(GetSymTP(FunctionBeingDefined))))) {
	    UserWarning(WARN_nonvoidreturn);
	}
    }
    /*
     * If there was a decl list before the compound statement, then this is
     * an old style declaration.
     */
    if (result) {
	CountFunctions++;
	Via(funcname)->Definition.FuncBody = result;
	Via(funcname)->storage_class = SCC_normal;
	if (NextIs(';'))
	    UserWarning(WARN_semiafterfunction);
    } else {
	if (tmp)
	    SyntaxError("Expected compound statement for function body");
    }
    FunctionBeingDefined = NULL;
    return result;
}
