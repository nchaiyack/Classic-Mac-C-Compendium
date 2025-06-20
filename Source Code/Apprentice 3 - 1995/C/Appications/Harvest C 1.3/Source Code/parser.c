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
 * This file contains the Harvest C parser.  The parser is hand written.
 * 
 * 
 */


#include "conditcomp.h"
#include <stdio.h>
#include <string.h>

#include "structs.h"

#pragma segment Parser


/* THEPARSER */

/*
 * Parsing:  We conceptually separate parsing into 2 stages : syntactic
 * parsing and semantic parsing.  During syntactic parsing, we wish to
 * identify all problems occuring in syntax only - such as unmatched parens.
 * During semantic parsing, we wish to identify all other problems which will
 * prohibit us from producing assembler output.  These problems include : use
 * of an undefined identifier, type mismatches, etc...
 LastToken
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

ParseTreeVia_t
Do_cast_expr(void)
{
    /*
     * cast_expr : unary_expr | '(' type_name ')' cast_expr ;
     */

    ParseTreeVia_t                  result;
    result = NULL;
    if (NextIs('(')) {
	TypeRecordVia_t                 tmp;
	tmp = Do_type_name();
	if (tmp) {
	    if (!NextIs(')')) {
		SyntaxError("Missing right parenthesis");
	    }
	    if (tmp) {
		result = Do_cast_expr();
		FuncPtrGenerate(&result);
		if (result) {
		    if (SameType(tmp, GetTreeTP(result))) {
			UserWarning(WARN_redundantcast);
		    }
		    result = TypeConvert(result, tmp);
		    CurrentSRC.CountCasts++;
		    Via(result)->LValue = 0;
		} else {
		    SyntaxError("Expected expr after type cast");
		}
	    } else {
		SyntaxError("Expected type name for cast");
	    }
	} else {
	    PutBackToken("(", '(');
	    result = NULL;
	}
    }
    if (!result) {
	result = Do_unary_expr();
    }
    return result;
}

/*
 * Here begin the arithmetic expressions.  Type checking for these is more
 * complicated.  Every operand must be checked for valid type. Rules must be
 * applied to ensure that the result type is correct. Handling of lvalue
 * status is important.
 */

ParseTreeVia_t
Do_multiplicative_expr(void)
{
    /*
     * multiplicative_expr : cast_expr | multiplicative_expr '*' cast_expr |
     * multiplicative_expr '/' cast_expr | multiplicative_expr '%' cast_expr
     * ;
     */

    /*
     * TYPERULE Both operands must be of arithmetic type.  The result type
     * depends on the types of the operands.  For the % operator, both
     * operands must be of integral type.
     */
    ParseTreeVia_t                  result;
    TypeRecordVia_t                 resulttype;
    ParseTreeVia_t                  right;
    int                             donemulting;
    result = Do_cast_expr();
    if (result) {
	donemulting = 0;
	while (!donemulting) {
	    if (NextIs('*')) {
		right = Do_cast_expr();
		if (!isArithmeticType(GetTreeTP(result))) {
		    TypeError("* (multiplication) requires arithmetic type");
		}
		if (right) {
		    if (!isArithmeticType(GetTreeTP(right))) {
			TypeError("* (multiplication) requires arithmetic type");
		    }
		    resulttype = Coerce(&result, &right, 0);
		    result = BuildTreeNode(PTF_multiply, result, right, NULL);
		    SetTreeTP(result, resulttype);
		    Via(result)->LValue = 0;
		} else {
		    SyntaxError("Expected expr as rhs of * (multiplication)");
		}
	    } else if (NextIs('/')) {
		right = Do_cast_expr();
		if (!isArithmeticType(GetTreeTP(result))) {
		    TypeError("/ requires arithmetic type");
		}
		if (right) {
		    if (!isArithmeticType(GetTreeTP(right))) {
			TypeError("/ requires arithmetic type");
		    }
		    resulttype = Coerce(&result, &right, 0);
		    result = BuildTreeNode(PTF_divide, result, right, NULL);
		    SetTreeTP(result, resulttype);
		    Via(result)->LValue = 0;
		} else {
		    SyntaxError("Expected expr as rhs of /");
		}
	    } else if (NextIs('%')) {
		right = Do_cast_expr();
		if (!isIntegralType(GetTreeTP(result))) {
		    TypeError("% requires integral type");
		}
		if (right) {
		    if (!isIntegralType(GetTreeTP(right))) {
			TypeError("% requires integral type");
		    }
		    resulttype = Coerce(&result, &right, 0);
		    result = BuildTreeNode(PTF_modulo, result, right, NULL);
		    SetTreeTP(result, resulttype);
		    Via(result)->LValue = 0;
		} else {
		    SyntaxError("Expected expr as rhs of %");
		}
	    } else {
		donemulting = 1;
	    }
	}
    }
    return result;
}

ParseTreeVia_t
Do_additive_expr(void)
{
    /*
     * additive_expr : multiplicative_expr | additive_expr '+'
     * multiplicative_expr | additive_expr '-' multiplicative_expr ;
     */

    ParseTreeVia_t                  result;
    TypeRecordVia_t                 resulttype;
    ParseTreeVia_t                  tmp;
    int                             doneadding;
    result = Do_multiplicative_expr();
    if (result) {
	doneadding = 0;
	while (!doneadding) {
	    if (NextIs('+')) {
		tmp = Do_multiplicative_expr();
		if (tmp) {
		    resulttype = Coerce(&result, &tmp, 0);
		    if (!isBooleanType(GetTreeTP(tmp))) {
			TypeError("+ requires arithmetic or pointer type");
		    }
		    if (!isBooleanType(GetTreeTP(result))) {
			TypeError("+ requires arithmetic or pointer type");
		    }
		    result = BuildTreeNode(PTF_add, result, tmp, NULL);
		    SetTreeTP(result, resulttype);
		    Via(result)->LValue = 0;
		} else {
		    SyntaxError("Expected expr as rhs of +");
		}
	    } else if (NextIs('-')) {
		tmp = Do_multiplicative_expr();
		if (!isBooleanType(GetTreeTP(result))) {
		    TypeError("- requires arithmetic or pointer type");
		}
		if (tmp) {
		    if (!isBooleanType(GetTreeTP(tmp))) {
			TypeError("- requires arithmetic or pointer type");
		    }
		    resulttype = Coerce(&result, &tmp, 0);
		    if (isPointerType(GetTreeTP(result)) && isPointerType(GetTreeTP(tmp))) {
			resulttype = BuildTypeRecord(0, TRC_long, SGN_signed);
		    }
		    result = BuildTreeNode(PTF_subtract, result, tmp, NULL);
		    SetTreeTP(result, resulttype);
		    Via(result)->LValue = 0;
		} else {
		    SyntaxError("Expected expr as rhs of -");
		}
	    } else {
		doneadding = 1;
	    }
	}
    }
    return result;
}

ParseTreeVia_t
Do_shift_expr(void)
{
    /*
     * shift_expr : additive_expr | shift_expr LEFT_OP additive_expr |
     * shift_expr RIGHT_OP additive_expr ;
     */

    ParseTreeVia_t                  result;
    TypeRecordVia_t                 resulttype;
    ParseTreeVia_t                  tmp;
    int                             doneshifting;
    result = Do_additive_expr();
    if (result) {
	doneshifting = 0;
	while (!doneshifting) {
	    if (NextIs(LEFT_OP)) {
		if (!isIntegralType(GetTreeTP(result))) {
		    TypeError("<< requires integral type");
		}
		tmp = Do_additive_expr();
		if (tmp) {
		    if (!isIntegralType(GetTreeTP(result))) {
			TypeError("<< requires integral type");
		    }
		    resulttype = Coerce(&result, &tmp, 0);
		    result = BuildTreeNode(PTF_shift_left, result, tmp, NULL);
		    SetTreeTP(result, resulttype);
		    Via(result)->LValue = 0;
		} else {
		    SyntaxError("Expected expr as rhs of <<");
		}
	    } else if (NextIs(RIGHT_OP)) {
		tmp = Do_additive_expr();
		if (!isIntegralType(GetTreeTP(result))) {
		    TypeError(">> requires integral type");
		}
		if (tmp) {
		    if (!isIntegralType(GetTreeTP(result))) {
			TypeError(">> requires integral type");
		    }
		    resulttype = Coerce(&result, &tmp, 0);
		    result = BuildTreeNode(PTF_shift_right, result, tmp, NULL);
		    SetTreeTP(result, resulttype);
		    Via(result)->LValue = 0;
		} else {
		    SyntaxError("Expected expr as rhs of >>");
		}
	    } else {
		doneshifting = 1;
	    }
	}
    }
    return result;
}

ParseTreeVia_t
Do_relational_expr(void)
{
    /*
     * relational_expr : shift_expr | relational_expr '<' shift_expr |
     * relational_expr '>' shift_expr | relational_expr LE_OP shift_expr |
     * relational_expr GE_OP shift_expr ;
     */

    ParseTreeVia_t                  result;
    ParseTreeVia_t                  tmp;
    int                             donerelating;
    result = Do_shift_expr();
    if (result) {
	donerelating = 0;
	while (!donerelating) {
	    if (NextIs('<')) {
		tmp = Do_shift_expr();
		if (!isBooleanType(GetTreeTP(result))) {
		    TypeError("< requires arithmetic or pointer type");
		}
		if (tmp) {
		    if (!isBooleanType(GetTreeTP(tmp))) {
			TypeError("< requires arithmetic or pointer type");
		    }
		    RelateCoerce(&result, &tmp);
		    result = BuildTreeNode(PTF_lessthan, result, tmp, NULL);
		    SetTreeTP(result, BuildTypeRecord(0, TRC_int, SGN_signed));
		    Via(result)->LValue = 0;
		} else {
		    SyntaxError("Expected expr as rhs of <");
		}
	    } else if (NextIs('>')) {
		tmp = Do_shift_expr();
		if (!isBooleanType(GetTreeTP(result))) {
		    TypeError("> requires arithmetic or pointer type");
		}
		if (tmp) {
		    if (!isBooleanType(GetTreeTP(tmp))) {
			TypeError("> requires arithmetic or pointer type");
		    }
		    RelateCoerce(&result, &tmp);
		    result = BuildTreeNode(PTF_greaterthan, result, tmp, NULL);
		    SetTreeTP(result, BuildTypeRecord(0, TRC_int, SGN_signed));
		    Via(result)->LValue = 0;
		} else {
		    SyntaxError("Expected expr as rhs of >");
		}
	    } else if (NextIs(LE_OP)) {
		tmp = Do_shift_expr();
		if (!isBooleanType(GetTreeTP(result))) {
		    TypeError("<= requires arithmetic or pointer type");
		}
		if (tmp) {
		    if (!isBooleanType(GetTreeTP(tmp))) {
			TypeError("<= requires arithmetic or pointer type");
		    }
		    RelateCoerce(&result, &tmp);
		    result = BuildTreeNode(PTF_lessthaneq, result, tmp, NULL);
		    SetTreeTP(result, BuildTypeRecord(0, TRC_int, SGN_signed));
		    Via(result)->LValue = 0;
		} else {
		    SyntaxError("Expected expr as rhs of <=");
		}
	    } else if (NextIs(GE_OP)) {
		tmp = Do_shift_expr();
		if (!isBooleanType(GetTreeTP(result))) {
		    TypeError(">= requires arithmetic or pointer type");
		}
		if (tmp) {
		    if (!isBooleanType(GetTreeTP(tmp))) {
			TypeError(">= requires arithmetic or pointer type");
		    }
		    RelateCoerce(&result, &tmp);
		    result = BuildTreeNode(PTF_greaterthaneq, result, tmp, NULL);
		    SetTreeTP(result, BuildTypeRecord(0, TRC_int, SGN_signed));
		    Via(result)->LValue = 0;
		} else {
		    SyntaxError("Expected expr as rhs of >=");
		}
	    } else {
		donerelating = 1;
	    }
	}
    }
    return result;
}

ParseTreeVia_t
Do_equality_expr(void)
{
    /*
     * equality_expr : relational_expr | equality_expr EQ_OP relational_expr
     * | equality_expr NE_OP relational_expr ;
     */

    ParseTreeVia_t                  result;
    ParseTreeVia_t                  tmp;
    result = Do_relational_expr();
    if (result) {
	if (NextIs(EQ_OP)) {
	    tmp = Do_relational_expr();
	    if (!isBooleanType(GetTreeTP(result))) {
		TypeError("== requires arithmetic or pointer type");
	    }
	    if (tmp) {
		if (!isBooleanType(GetTreeTP(tmp))) {
		    TypeError("== requires arithmetic or pointer type");
		}
		(void) Coerce(&result, &tmp, 0);
		if (isFloatingType(GetTreeTP(result))) {
		    UserWarning(WARN_floateqcompare);
		}
		result = BuildTreeNode(PTF_equal, result, tmp, NULL);
		SetTreeTP(result, BuildTypeRecord(0, TRC_int, SGN_signed));
		Via(result)->LValue = 0;
	    } else {
		SyntaxError("Expected expr as rhs of ==");
	    }
	} else if (NextIs(NE_OP)) {
	    tmp = Do_relational_expr();
	    if (!isBooleanType(GetTreeTP(result))) {
		TypeError("!= requires arithmetic or pointer type");
	    }
	    if (tmp) {
		if (!isBooleanType(GetTreeTP(tmp))) {
		    TypeError("!= requires arithmetic or pointer type");
		}
		(void) Coerce(&result, &tmp, 0);
		if (isFloatingType(GetTreeTP(result))) {
		    UserWarning(WARN_floateqcompare);
		}
		result = BuildTreeNode(PTF_notequal, result, tmp, NULL);
		SetTreeTP(result, BuildTypeRecord(0, TRC_int, SGN_signed));
		Via(result)->LValue = 0;
	    } else {
		SyntaxError("Expected expr as rhs of !=");
	    }
	}
    }
    return result;
}

ParseTreeVia_t
Do_and_expr(void)
{
    /*
     * and_expr : equality_expr | and_expr '&' equality_expr ;
     */

    ParseTreeVia_t                  result;
    TypeRecordVia_t                 resulttype;
    ParseTreeVia_t                  tmp;
    int                             doneanding;
    result = Do_equality_expr();
    if (result) {
	doneanding = 0;
	while (!doneanding) {
	    if (NextIs('&')) {
		tmp = Do_equality_expr();
		if (!isIntegralType(GetTreeTP(result))) {
		    TypeError("& (bitwise and) requires integral type");
		}
		if (tmp) {
		    if (!isIntegralType(GetTreeTP(tmp))) {
			TypeError("& (bitwise and) requires integral type");
		    }
		    resulttype = Coerce(&result, &tmp, 0);
		    result = BuildTreeNode(PTF_bitwise_and, result, tmp, NULL);
		    SetTreeTP(result, resulttype);
		    Via(result)->LValue = 0;
		} else {
		    SyntaxError("Expected expr as rhs of &");
		}
	    } else {
		doneanding = 1;
	    }
	}
    }
    return result;
}

ParseTreeVia_t
Do_exclusive_or_expr(void)
{
    /*
     * exclusive_or_expr : and_expr | exclusive_or_expr '^' and_expr ;
     */

    ParseTreeVia_t                  result;
    TypeRecordVia_t                 resulttype;
    ParseTreeVia_t                  tmp;
    int                             doneexoring;
    result = Do_and_expr();
    if (result) {
	doneexoring = 0;
	while (!doneexoring) {
	    if (NextIs('^')) {
		tmp = Do_and_expr();
		if (!isIntegralType(GetTreeTP(result))) {
		    TypeError("^ requires integral type");
		}
		if (tmp) {
		    if (!isIntegralType(GetTreeTP(tmp))) {
			TypeError("^ requires integral type");
		    }
		    resulttype = Coerce(&result, &tmp, 0);
		    result = BuildTreeNode(PTF_bitwise_xor, result, tmp, NULL);
		    SetTreeTP(result, resulttype);
		    Via(result)->LValue = 0;
		} else {
		    SyntaxError("Expected expr as rhs of ^");
		}
	    } else {
		doneexoring = 1;
	    }
	}
    }
    return result;
}

ParseTreeVia_t
Do_inclusive_or_expr(void)
{
    /*
     * inclusive_or_expr : exclusive_or_expr | inclusive_or_expr '|'
     * exclusive_or_expr ;
     */

    ParseTreeVia_t                  result;
    TypeRecordVia_t                 resulttype;
    ParseTreeVia_t                  tmp;
    int                             doneinoring;
    result = Do_exclusive_or_expr();
    if (result) {
	doneinoring = 0;
	while (!doneinoring) {
	    if (NextIs('|')) {
		tmp = Do_exclusive_or_expr();
		if (!isIntegralType(GetTreeTP(result))) {
		    TypeError("| requires integral type");
		}
		if (tmp) {
		    if (!isIntegralType(GetTreeTP(tmp))) {
			TypeError("| requires integral type");
		    }
		    resulttype = Coerce(&result, &tmp, 0);
		    result = BuildTreeNode(PTF_bitwise_or, result, tmp, NULL);
		    SetTreeTP(result, resulttype);
		    Via(result)->LValue = 0;
		} else {
		    SyntaxError("Expected expr as rhs of |");
		}
	    } else {
		doneinoring = 1;
	    }
	}
    }
    return result;
}

ParseTreeVia_t
Do_logical_and_expr(void)
{
    /*
     * logical_and_expr : inclusive_or_expr | logical_and_expr AND_OP
     * inclusive_or_expr ;
     */

    ParseTreeVia_t                  result;
    ParseTreeVia_t                  tmp;
    int                             donelogicaland;
    result = Do_inclusive_or_expr();
    if (result) {
	donelogicaland = 0;
	while (!donelogicaland) {
	    if (NextIs(AND_OP)) {
		tmp = Do_inclusive_or_expr();
		if (!isBooleanType(GetTreeTP(result))) {
		    TypeError("&& requires arithmetic or pointer type");
		}
		if (tmp) {
		    if (!isBooleanType(GetTreeTP(tmp))) {
			TypeError("&& requires arithmetic or pointer type");
		    }
		    /* QQQQ Can I really delete this ? */
#ifdef UNDEFINED
		    (void) Coerce(&result, &tmp, 1);
#endif
		    result = BuildTreeNode(PTF_logical_and, result, tmp, NULL);
		    SetTreeTP(result, BuildTypeRecord(0, TRC_int, SGN_signed));
		    Via(result)->LValue = 0;
		} else {
		    SyntaxError("Expected expr as rhs of &&");
		}
	    } else {
		donelogicaland = 1;
	    }
	}
    }
    return result;
}

ParseTreeVia_t
Do_logical_or_expr(void)
{
    /*
     * logical_or_expr : logical_and_expr | logical_or_expr OR_OP
     * logical_and_expr ;
     */

    ParseTreeVia_t                  result;
    ParseTreeVia_t                  tmp;
    int                             donelogicalor;
    result = Do_logical_and_expr();
    if (result) {
	donelogicalor = 0;
	while (!donelogicalor) {
	    if (NextIs(OR_OP)) {
		tmp = Do_logical_and_expr();
		if (!isBooleanType(GetTreeTP(result))) {
		    TypeError("|| requires arithmetic or pointer type");
		}
		if (tmp) {
		    if (!isBooleanType(GetTreeTP(tmp))) {
			TypeError("|| requires arithmetic or pointer type");
		    }
		    /* QQQQ Can I really delete this ? */
#ifdef UNDEFINED
		    (void) Coerce(&result, &tmp, 1);
#endif
		    result = BuildTreeNode(PTF_logical_or, result, tmp, NULL);
		    SetTreeTP(result, BuildTypeRecord(0, TRC_int, SGN_signed));
		    Via(result)->LValue = 0;
		} else {
		    SyntaxError("Expected expr as rhs of ||");
		}
	    } else {
		donelogicalor = 1;
	    }
	}
    }
    return result;
}

ParseTreeVia_t
Do_conditional_expr(void)
{
    /*
     * conditional_expr : logical_or_expr | logical_or_expr '?'
     * logical_or_expr ':' conditional_expr ;
     */

    ParseTreeVia_t                  result;
    TypeRecordVia_t                 resulttype;
    ParseTreeVia_t                  tmp2;
    result = Do_logical_or_expr();
    if (result) {
	if (NextIs('?')) {
	    ParseTreeVia_t                  tmp;
	    tmp = Do_logical_or_expr();
	    if (!isBooleanType(GetTreeTP(result))) {
		TypeError("Left operand of ? operator must be arithmetic or pointer");
	    }
	    if (tmp) {
		if (NextIs(':')) {
		    tmp2 = Do_conditional_expr();
		    if (tmp2) {
			resulttype = Coerce(&tmp, &tmp2, 0);
			if (!SameType(GetTreeTP(tmp), GetTreeTP(tmp2))) {
			    TypeError("Middle and rightmost operands of ?: must be of same type.");
			}
			result = BuildTreeNode(PTF_ternary, result, tmp, tmp2);
			SetTreeTP(result, resulttype);
			Via(result)->LValue = 0;
		    } else {
			SyntaxError("Expected 3rd expr for ternary");
		    }
		} else {
		    SyntaxError("Expected : following ?");
		}
	    } else {
		SyntaxError("Expected middle expr for ternary");
	    }
	}
    }
    return result;
}

ParseTreeVia_t
Do_assignment_expr(void)
{
    /*
     * assignment_expr : conditional_expr | unary_expr assignment_operator
     * assignment_expr ;
     */
    /*
     * This routine is the parser to call for reading an expression where
     * commas are not legal (such as in function arguments).
     */
    ParseTreeVia_t                  result;
    Codigo_t                        tmp;
    ParseTreeVia_t                  tmp2;

    result = Do_conditional_expr();
    if (result) {
	PtrGenerate(&result);
	tmp = Do_assignment_operator();
	if (tmp) {
	    if (!Via(result)->LValue) {
		SemanticError("Assignment requires an lvalue on left");
	    }
	    if (GetTPQual(GetTreeTP(result)) == SCC_const) {
		SemanticError("No assignment to const types");
	    }
	    tmp2 = Do_assignment_expr();
	    if (tmp2) {
		(void) AssignCoerce(&result, &tmp2);
		if (!SameType(GetTreeTP(result), GetTreeTP(tmp2))) {
		    TypeError("Incompatible types for assignment");
		}
		switch (tmp) {
		case '=':
		    result = BuildTreeNode(PTF_assign, result, tmp2, NULL);
		    SetTreeTP(result, GetTreeTP(tmp2));
		    Via(result)->LValue = 0;
		    break;
		case MUL_ASSIGN:
		    result = BuildTreeNode(PTF_mulassign, result, tmp2, NULL);
		    SetTreeTP(result, GetTreeTP(tmp2));
		    Via(result)->LValue = 0;
		    break;
		case DIV_ASSIGN:
		    result = BuildTreeNode(PTF_divassign, result, tmp2, NULL);
		    SetTreeTP(result, GetTreeTP(tmp2));
		    Via(result)->LValue = 0;
		    break;
		case MOD_ASSIGN:
		    if (isFloatingType(GetTreeTP(result))) {
			TypeError("%= requires integral type");
		    }
		    result = BuildTreeNode(PTF_modassign, result, tmp2, NULL);
		    SetTreeTP(result, GetTreeTP(tmp2));
		    Via(result)->LValue = 0;
		    break;
		case ADD_ASSIGN:
		    result = BuildTreeNode(PTF_addassign, result, tmp2, NULL);
		    SetTreeTP(result, GetTreeTP(tmp2));
		    Via(result)->LValue = 0;
		    break;
		case SUB_ASSIGN:
		    result = BuildTreeNode(PTF_subassign, result, tmp2, NULL);
		    SetTreeTP(result, GetTreeTP(tmp2));
		    Via(result)->LValue = 0;
		    break;
		case LEFT_ASSIGN:
		    if (isFloatingType(GetTreeTP(result))) {
			TypeError("<<= requires integral type");
		    }
		    result = BuildTreeNode(PTF_leftassign, result, tmp2, NULL);
		    SetTreeTP(result, GetTreeTP(tmp2));
		    Via(result)->LValue = 0;
		    break;
		case RIGHT_ASSIGN:
		    if (isFloatingType(GetTreeTP(result))) {
			TypeError(">>= requires integral type");
		    }
		    result = BuildTreeNode(PTF_rightassign, result, tmp2, NULL);
		    SetTreeTP(result, GetTreeTP(tmp2));
		    Via(result)->LValue = 0;
		    break;
		case AND_ASSIGN:
		    if (isFloatingType(GetTreeTP(result))) {
			TypeError("&= requires integral type");
		    }
		    result = BuildTreeNode(PTF_andassign, result, tmp2, NULL);
		    SetTreeTP(result, GetTreeTP(tmp2));
		    Via(result)->LValue = 0;
		    break;
		case XOR_ASSIGN:
		    if (isFloatingType(GetTreeTP(result))) {
			TypeError("^= requires integral type");
		    }
		    result = BuildTreeNode(PTF_xorassign, result, tmp2, NULL);
		    SetTreeTP(result, GetTreeTP(tmp2));
		    Via(result)->LValue = 0;
		    break;
		case OR_ASSIGN:
		    if (isFloatingType(GetTreeTP(result))) {
			TypeError("|= requires integral type");
		    }
		    result = BuildTreeNode(PTF_orassign, result, tmp2, NULL);
		    SetTreeTP(result, GetTreeTP(tmp2));
		    Via(result)->LValue = 0;
		    break;
		default:
		    break;
		}
	    } else {
		SyntaxError("Expected expr for rhs of = (assignment)");
	    }
	}
    }
    return result;
}

Codigo_t
Do_assignment_operator(void)
{
    /*
     * assignment_operator : '=' | MUL_ASSIGN | DIV_ASSIGN | MOD_ASSIGN |
     * ADD_ASSIGN | SUB_ASSIGN | LEFT_ASSIGN | RIGHT_ASSIGN | AND_ASSIGN |
     * XOR_ASSIGN | OR_ASSIGN ;
     */

    if (NextIs('=')) {
	return '=';
    } else if (NextIs(MUL_ASSIGN)) {
	return MUL_ASSIGN;
    } else if (NextIs(DIV_ASSIGN)) {
	return DIV_ASSIGN;
    } else if (NextIs(MOD_ASSIGN)) {
	return MOD_ASSIGN;
    } else if (NextIs(ADD_ASSIGN)) {
	return ADD_ASSIGN;
    } else if (NextIs(SUB_ASSIGN)) {
	return SUB_ASSIGN;
    } else if (NextIs(LEFT_ASSIGN)) {
	return LEFT_ASSIGN;
    } else if (NextIs(RIGHT_ASSIGN)) {
	return RIGHT_ASSIGN;
    } else if (NextIs(AND_ASSIGN)) {
	return AND_ASSIGN;
    } else if (NextIs(XOR_ASSIGN)) {
	return XOR_ASSIGN;
    } else if (NextIs(OR_ASSIGN)) {
	return OR_ASSIGN;
    } else {
	return 0;
    }
}

ParseTreeVia_t
Do_expr(void)
{
    /*
     * expr : assignment_expr | expr ',' assignment_expr ;
     */
    /*
     * This is the parser routine to call for reading an expression. It
     * returns an expression parse tree.  The expression parsed should be one
     * wherein commas are legal.
     */

    ParseTreeVia_t                  result;
    ParseTreeVia_t                  tmp;
    int                             doneexpring;
    result = Do_assignment_expr();
    if (result) {
	doneexpring = 0;
	while (!doneexpring) {
	    if (NextIs(',')) {
		tmp = Do_assignment_expr();
		if (tmp) {
		    result = BuildTreeNode(PTF_commas, result, tmp, NULL);
		    SetTreeTP(result, GetTreeTP(tmp));
		    Via(result)->LValue = 0;
		} else {
		    SyntaxError("Expected expr following comma");
		}
	    } else {
		doneexpring = 1;
	    }
	}
    }
    return result;
}

ParseTreeVia_t
Do_constant_expr(void)
{
    /*
     * constant_expr : conditional_expr ;
     */
    /*
     * There are semantic restrictions as to what may appear in a constant
     * expression.  They will be verified using the is_constant field of the
     * expr struct.
     */

    ParseTreeVia_t                  result;
    result = Do_conditional_expr();
    return result;
}
