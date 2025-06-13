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
 * This file contains the Harvest C parser.  The parser is hand written,
 * recursive descent.
 * 
 * 
 */


#include "conditcomp.h"
#include <stdio.h>
#include <string.h>
#include "structs.h"

#include "CHarvestDoc.h"
#include "CHarvestOptions.h"

extern CHarvestDoc *gProject;

#pragma segment Parse1

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

FloatLitVia_t
RawFloatLit(void)
{
    register FloatLitVia_t          raw;
    raw = (FloatLitVia_t) Ealloc(sizeof(FloatLit_t));
    Via(raw)->next = NULL;
    Via(raw)->FloatFlags = 0;
    Via(raw)->Loc = BuildARegLabelDisplace(5, MakeFloatLitLabel(NextFloatLitLabel++));
    SetLocSZ(Via(raw)->Loc, M68_TypeSize(BuildTypeRecord(0, TRC_longdouble, SGN_unknown)));
    SetLocIsFloat(Via(raw)->Loc, 1);
    Via(raw)->val.num = 0;
    return raw;
}

FloatLitVia_t
AddFloatLit(long double x)
{
    FloatLitVia_t                   temp;
    temp = RawFloatLit();
    Via(temp)->next = FloatList;
    Via(temp)->val.num = x;
    FloatList = temp;
    return FloatList;
}

void
KillFloatList(FloatLitVia_t head)
{
    if (head) {
	KillFloatList(Via(head)->next);
	Efree(head);
    }
}

int
GetIntValue(ParseTreeVia_t root)
/* Returns the integer value of a constant expression tree. */
{
    int                             result;
    result = 0;
    if (root) {
	switch (Via(root)->kind) {
	case PTF_identifier:
	    /*
	     * This could probably be more intelligent... I am doing it this
	     * way for the preprocessor...
	     */
	    result = 0;
	    break;
	case PTF_intconstant:
	    result = Via(root)->data.number;
	    break;
	case PTF_typechange:
	case PTF_unary_plus:
	    result = GetIntValue(Via(root)->a);
	    break;
	case PTF_unary_minus:
	    result = 0 - GetIntValue(Via(root)->a);
	    break;
	case PTF_enumconstant:
	    result = Via(Via(root)->data.thesymbol)->numbers.EnumVal;
	    break;
	case PTF_sizeof:
	    result = GetTPSize(Via(root)->data.TP);
	    break;
	case PTF_logical_and:
	    result = GetIntValue(Via(root)->a) && GetIntValue(Via(root)->b);
	    break;
	case PTF_logical_neg:
	    result = !GetIntValue(Via(root)->a);
	    break;
	case PTF_multiply:
	    result = GetIntValue(Via(root)->a) * GetIntValue(Via(root)->b);
	    break;
	case PTF_divide:
	    result = GetIntValue(Via(root)->a) / GetIntValue(Via(root)->b);
	    break;
	case PTF_modulo:
	    result = GetIntValue(Via(root)->a) % GetIntValue(Via(root)->b);
	    break;
	case PTF_add:
	    result = GetIntValue(Via(root)->a) + GetIntValue(Via(root)->b);
	    break;
	case PTF_subtract:
	    result = GetIntValue(Via(root)->a) - GetIntValue(Via(root)->b);
	    break;
	case PTF_shift_left:
	    result = GetIntValue(Via(root)->a) << GetIntValue(Via(root)->b);
	    break;
	case PTF_shift_right:
	    result = GetIntValue(Via(root)->a) >> GetIntValue(Via(root)->b);
	    break;
	case PTF_lessthan:
	    result = GetIntValue(Via(root)->a) < GetIntValue(Via(root)->b);
	    break;
	case PTF_greaterthan:
	    result = GetIntValue(Via(root)->a) > GetIntValue(Via(root)->b);
	    break;
	case PTF_lessthaneq:
	    result = GetIntValue(Via(root)->a) <= GetIntValue(Via(root)->b);
	    break;
	case PTF_greaterthaneq:
	    result = GetIntValue(Via(root)->a) >= GetIntValue(Via(root)->b);
	    break;
	case PTF_logical_or:
	    result = GetIntValue(Via(root)->a) || GetIntValue(Via(root)->b);
	    break;
	    /* TODO There are probably more legal possibilities here. */
	case PTF_postincrement:
	case PTF_postdecrement:
	case PTF_preincrement:
	case PTF_predecrement:
	case PTF_assign:
	case PTF_mulassign:
	case PTF_divassign:
	case PTF_modassign:
	case PTF_addassign:
	case PTF_subassign:
	case PTF_leftassign:
	case PTF_rightassign:
	case PTF_andassign:
	case PTF_xorassign:
	case PTF_orassign:
	    SyntaxError("Side effects not allowed in constant expressions");
	    break;
	default:
	    result = 0;
	    VeryBadParseError("Unrecognized expression in GetIntValue");
	    break;
	}
    }
    return result;
}

/* EXPR__ */
int
GetIDENTIFIER(void)
{
    if (NextIs(IDENTIFIER)) {
	return 1;
    } else if (NextIs(TYPEDEF_NAME)) {
	return 2;
    } else {
	return 0;
    }
}

/*
 * The functions for the recursive descent parser appear below. This parser
 * is based on the ANSI grammar, appearing in K&R II.  I received a YACC
 * version of this grammar, which appears to be identical, from John Levine,
 * moderator of comp.compilers.  Levine can be reached at
 * compilers-request@iecc@cambridge.ma.us; I have a grammar from James
 * Roskind, which may be better than the ANSI one.  Roskind's grammar handles
 * redef of typedef names and I know that it has been extensively tested and
 * debugged recently. Roskind is jar@ileaf.com.  Roskind's grammar was not
 * used for this compiler.
 */

int
CountArgs(ParseTreeVia_t list)
{
    if (Via(list)->kind == PTF_argument_list) {
	return 1 + CountArgs(Via(list)->b);
    } else {
	return 1;
    }
}

void
ProtoTypeCheck(SymListVia_t prot,
	       ParseTreeVia_t * carg,
	       int isellipsis)
{
    SYMVia_t                        param;
    ParseTreeVia_t                  prevcarg;
    int                             done, iscount, sbcount, ndx;
    done = 0;
    iscount = 0;
    sbcount = Via(prot)->count;
    ndx = CountArgs(*carg);
    while (!done) {
	prevcarg = *carg;
	if (Via((*carg))->kind == PTF_argument_list) {
	    /* Check Via(carg)->a against item ndx in prot */
	    param = NULL;
	    if (ndx <= Via(prot)->count)
		param = TableGetNum(prot, ndx);
	    if (param) {
		CallCoerce(GetSymTP(param), &(Via((*carg))->a));
	    } else {
		assert(isellipsis);
		PtrGenerate(&(Via((*carg))->a));
		StandardIntPromote(&(Via((*carg))->a));
		StandardFloatPromote(&(Via((*carg))->a));
	    }
	    carg = &((Via(prevcarg))->b);	/* Next arg. */
	    ndx--;
	} else {
	    /* Last arg. */
	    /* Check carg against item ndx in prot */
	    param = NULL;
	    if (ndx <= Via(prot)->count)
		param = TableGetNum(prot, ndx);
	    if (param) {
		CallCoerce(GetSymTP(param), carg);
	    } else {
		assert(isellipsis);
		PtrGenerate(carg);
		StandardIntPromote(carg);
		StandardFloatPromote(carg);
	    }
	    done = 1;
	}
	iscount++;
    }
    if (!isellipsis) {
	if (iscount != sbcount) {
	    SemanticError("Wrong number of args to function");
	}
    }
}

int
GetMessageKey(void)
{
    return GetIDENTIFIER();	/* TODO needs more */
}

ParseTreeVia_t
Do_postfix_expr(void)
{
    /*
     * primary_expr : IDENTIFIER | message send | INTCONSTANT | FLOATCONSTANT
     * | STRING_LITERAL | '(' expr ')' ;
     */

    /*
     * postfix_expr : primary_expr | postfix_expr '[' expr ']' | postfix_expr
     * '(' ')' | postfix_expr '(' argument_expr_list ')' | postfix_expr '.'
     * IDENTIFIER | postfix_expr PTR_OP IDENTIFIER | postfix_expr INC_OP |
     * postfix_expr DEC_OP ;
     */

    /*
     * I have merged primary expr and postfix expr to form a single routine.
     * Type checking of identifiers will occur here.  The procedure is as
     * follows : for INTCONSTANT (CHARCONSTANT), FLOATCONSTANT,
     * STRING_LITERAL, assign the type based on what is known (STRING_LITERAL
     * is trivial). For IDENTIFIER, look up its type (if it is available, no
     * problem). If its type can not be found, then check the next token.  If
     * it is a '(', then do an implicit declaration.  Be sure to putback the
     * paren, or handle the whole function call. Note that the routine is
     * broken into two halves. Even though I merged the two, I did not
     * integrate them well.
     */

    ParseTreeVia_t                  result;
    ParseTreeVia_t                  tmp;
    int                             donepostfixing;
    TypeRecordVia_t                 resulttype;
    SYMVia_t                        ident;
    int                             LeftUndeclared;
    char                            idfound[128];
    int                             ndx;
    int                             done;
    ParseTreeVia_t                 *carg;
    ParseTreeVia_t                  prevcarg;
    int                             iscount;
    LeftUndeclared = 0;
    /*
     * This identifier COULD also be a typedefname, iff that typedef name is
     * in the current local symbol table.
     */
    if (GetIDENTIFIER()) {
	ident = LookUpSymbol(LastToken);
	if (ident) {
	    if (Via(ident)->storage_class == SCC_enum) {
		result = BuildTreeNode(PTF_enumconstant, NULL, NULL, NULL);
		SetTreeTP(result, BuildTypeRecord(0, TRC_int, SGN_signed));
		SetTreeLValue(result, 0);
		Via(result)->data.thesymbol = ident;
	    } else {
		Via(ident)->numbers.CountUses++;
		result = BuildTreeNode(PTF_identifier, NULL, NULL, NULL);
		SetTreeTP(result, GetSymTP(ident));
		if (GetTreeTP(result)) {
		    SetTreeLValue(result, StartLValue(GetTreeTP(result)));
		}
		Via(result)->data.thesymbol = ident;
	    }
	} else {
	    strcpy(idfound, LastToken);
	    LeftUndeclared = 1;
	    result = BuildTreeNode(PTF_identifier, NULL, NULL, NULL);
	    SetTreeTP(result, 0);
	    SetTreeLValue(result, 0);
	    Via(result)->data.thesymbol = NULL;
	}
    } else if (NextIs(CHARCONSTANT)) {	/* Note that int and char constants
					 * are handled identically. */
	result = BuildTreeNode(PTF_intconstant, NULL, NULL, NULL);
	SetTreeTP(result, 0);
	SetTreeLValue(result, 0);
	Via(result)->data.number = LastIntegerConstant;
	if (strlen(LastToken) > 2) {
	    SetTreeTP(result, BuildTypeRecord(0, TRC_long, SGN_unsigned));
	}
	if (!(GetTreeTP(result)) && (strlen(LastToken) > 1)) {
	    SetTreeTP(result, BuildTypeRecord(0, TRC_int, SGN_unsigned));
	}
	if (!GetTreeTP(result)) {
	    if (gProject->itsOptions->signedChars) {
		SetTreeTP(result, BuildTypeRecord(0, TRC_char, SGN_signed));
	    } else {
		SetTreeTP(result, BuildTypeRecord(0, TRC_char, SGN_unsigned));
	    }
	}
    } else if (NextIs(INTCONSTANT)) {
	int                             sufndx;
	int                             YU, YL;
	YU = YL = 0;
	result = BuildTreeNode(PTF_intconstant, NULL, NULL, NULL);
	SetTreeLValue(result, 0);
	Via(result)->data.number = LastIntegerConstant;
	sufndx = 0;
	while (LastToken[sufndx]) {
	    switch (LastToken[sufndx]) {
	    case 'U':
		YU = 1;
		break;
	    case 'L':
		YL = 1;
		break;
	    default:
		break;
	    }
	    sufndx++;
	}
	if (YU) {
	    if (YL) {
		SetTreeTP(result, BuildTypeRecord(0, TRC_long, SGN_unsigned));
	    } else {
		SetTreeTP(result, BuildTypeRecord(0, TRC_int, SGN_unsigned));
	    }
	} else {
	    if (YL) {
		SetTreeTP(result, BuildTypeRecord(0, TRC_long, SGN_signed));
	    } else {
		SetTreeTP(result, BuildTypeRecord(0, TRC_int, SGN_signed));
	    }
	}
    } else if (NextIs(FLOATCONSTANT)) {
	FloatLitVia_t                   lit;
	result = BuildTreeNode(PTF_floatconstant, NULL, NULL, NULL);
	SetTreeTP(result, BuildTypeRecord(0, TRC_longdouble, SGN_unknown));
	SetTreeLValue(result, 0);
	lit = AddFloatLit(LastFloatingConstant);
	Via(result)->data.FLit = lit;
    } else if (NextIs(STRING_LITERAL)) {
		SYMVia_t                        lit;
		result = BuildTreeNode(PTF_string_literal, NULL, NULL, NULL);
		lit = TableSearch(StringLits, (LastToken));
		if (lit) {
		    DuplicateStringLits++;
		} else {
		    lit = TableAdd(StringLits, (LastToken));
		}
		if (!(Via(lit)->M68kDef.Loc)) {
		    Via(lit)->M68kDef.Loc = BuildARegLabelDisplace(5, MakeLitLabel(NextLitLabel++));
		}
		MakeLocGlobal(Via(lit)->M68kDef.Loc);
		Via(result)->data.SLit = lit;
		/* The type of this expression is array of char. */
		/* We immediately do the pointer generation to char * */
		if (gProject->itsOptions->signedChars) {
		    SetTreeTP(result, BuildTypeRecord(0, TRC_char, SGN_signed));
		} else {
		    SetTreeTP(result, BuildTypeRecord(0, TRC_char, SGN_unsigned));
		}
		SetTreeTP(result, BuildTypeRecord(GetTreeTP(result), TRC_pointer, SGN_unknown));
		SetTreeLValue(result, 0);
    } else if (NextIs(PASCSTRING_LITERAL)) {
		SYMVia_t                        lit;
		result = BuildTreeNode(PTF_string_literal, NULL, NULL, NULL);
		lit = TableSearch(StringLits, (LastToken));
		if (lit) {
		    DuplicateStringLits++;
		} else {
		    lit = TableAdd(StringLits, (LastToken));
		}
		Via(lit)->SymbolFlags |= ISPASCALSTRING;
		if (!(Via(lit)->M68kDef.Loc))
		    Via(lit)->M68kDef.Loc = BuildARegLabelDisplace(5, MakeLitLabel(NextLitLabel++));
		MakeLocGlobal(Via(lit)->M68kDef.Loc);
		Via(result)->data.SLit = lit;
		/* The type of this expression is array of char. */
		/* We immediately do the pointer generation to char * */
		if (gProject->itsOptions->signedChars) {
		    SetTreeTP(result, BuildTypeRecord(0, TRC_char, SGN_signed));
		} else {
		    SetTreeTP(result, BuildTypeRecord(0, TRC_char, SGN_unsigned));
		}
		SetTreeTP(result, BuildTypeRecord(GetTreeTP(result), TRC_pointer, SGN_unknown));
		SetTreeLValue(result, 0);
    } else if (NextIs('(')) {
	result = Do_expr();
	/*
	 * No modification of the type or lvalue status of the resulting
	 * expression is necessary.
	 */
	if (!NextIs(')')) {
	    SyntaxError("Missing right parenthesis");
	}
    } else {
	result = NULL;
    }

    /* The above portion is do_primary_expr() */

    if (result) {
	donepostfixing = 0;
	while (!donepostfixing) {
	    FetchToken();
	    switch (LastTokenKind) {
	    case '[':
		if (LeftUndeclared) {
		    SemanticError2("Undeclared identifier : ", idfound);
		    ident = TableAdd(GlobalSymbolTable, (idfound));
		    SetSymTP(ident, BuildTypeRecord(0, TRC_int, SGN_signed));
		    Via(ident)->numbers.CountUses++;
		    LeftUndeclared = 0;
		    SetTreeTP(result, GetSymTP(ident));
		    SetTreeLValue(result, 0);
		    Via(result)->data.thesymbol = ident;
		}
		tmp = Do_expr();
		if (NextIs(']')) {
		    if (tmp) {
			PtrGenerate(&result);
			PtrGenerate(&tmp);
			if ((resulttype = (isPointerType(GetTreeTP(result)))) != 0) {
			    if (!isIntegralType(GetTreeTP(tmp))) {
				TypeError("Array subscript here must be of integral type");
			    }
			} else {
			    if ((resulttype = isPointerType(GetTreeTP(tmp))) != 0) {
				if (!isIntegralType(GetTreeTP(result))) {
				    TypeError("Array subscript here must be pointer type");
				}
			    } else {
				TypeError("Either array base or subscript must be a pointer.");
			    }
			}
			StandardIntPromote(&result);
			StandardIntPromote(&tmp);
			result = BuildTreeNode(PTF_array_subscript, result, tmp, NULL);
			SetTreeTP(result, resulttype);
			if (!isArrayType(resulttype)) {
			    SetTreeLValue(result, 1);
			} else {
			    SetTreeLValue(result, 0);
			}
			/*
			 * TYPERULE We have two expressions, result, and tmp.
			 * One of these two must have type pointer to X, and
			 * the other must have integral type. The resulting
			 * postfix expression has type X. Note that according
			 * to K&R ][, section A7.1, Pointer Generation, an
			 * expression of type "array of X", is converted to
			 * type "pointer to X", and the value of the
			 * expression is the pointer to the first element in
			 * the array. Such a type conversion does not take
			 * place if the expression is the operand of the
			 * unary & operator, or of ++, --, sizeof, or as the
			 * left operand of an assignment operator, or the .
			 * operator.
			 */
			/*
			 * QQQQ WHEN and WHERE should the implementation of
			 * Pointer Generation, K&R A7.1, take place ?
			 */
		    } else {
			SyntaxError("Expecting array subscript expression");
		    }
		} else {
		    SyntaxError("Missing right bracket");
		}
		break;
	    case '(':{
		    /*
		     * TYPERULE The expr in result must have type pointer to
		     * function returning X, and the resulting postfix expr
		     * has type X.  Note that according to K&R ][, section
		     * A7.1, Pointer Generation, an expression of type
		     * "function returning X", except when used as the
		     * operand of the "address of" operator, is converted to
		     * type "pointer to function returning X."  The result is
		     * not an lvalue.
		     */
		    TypeRecordVia_t                 ftype;
		    TypeRecordVia_t                 functype;
		    SymListVia_t                    prot;
		    if (LeftUndeclared) {
				UserWarning2(WARN_implicitdecl, idfound);
				if (gProject->itsOptions->requireProtos) {
				    TypeError("Prototypes required");
				}
				ident = TableAdd(CurrentSymbols(), idfound);
				SetSymTP(ident, BuildTypeRecord(0, TRC_int, SGN_signed));
				Via(ident)->storage_class = SCC_extern;
				Via(ident)->numbers.CountUses++;
				SetSymTP(ident, BuildTypeRecord(GetSymTP(ident), TRC_OLDfunction, SGN_unknown));
				if (gProject->itsOptions->bigGlobals) {
				    Via(ident)->M68kDef.Loc = BuildLargeGlobal(MakeUserLabel(idfound));
				    MakeLocGlobal(Via(ident)->M68kDef.Loc);
				} else {
				    Via(ident)->M68kDef.Loc = BuildARegLabelDisplace(5, MakeUserLabel(idfound));
				    MakeLocGlobal(Via(ident)->M68kDef.Loc);
				}
			    SetLocSZ(Via(ident)->M68kDef.Loc, M68_TypeSize(Via(ident)->TP));
				LeftUndeclared = 0;
				SetTPMembers(GetSymTP(ident), RawTable(11));
				SetTreeTP(result, GetSymTP(ident));
				SetTreeLValue(result, 0);
				Via(result)->data.thesymbol = ident;
		    }
		    /*
		     * Here, if result is holding an expression of the form
		     * (*fp), then this is a call thru a function pointer,
		     * and the user has specified indirection (pre ANSI).  We
		     * have to get rid of the indirection.
		     */
		    if (Via(result)->kind == PTF_deref) {
			ParseTreeVia_t                  tmp;
			tmp = result;
			result = Via(result)->a;
			Via(tmp)->a = NULL;
			FreeTree(tmp);
		    }
		    ftype = functype = GetFuncType(GetTreeTP(result));
		    if (!functype) {
			ftype = functype = GetFuncType(isPointerType(GetTreeTP(result)));
		    }
		    assert(functype);
		    prot = GetTPMembers(ftype);
		    if (!(resulttype = isFunctionType(isPointerType(GetTreeTP(result))))) {
			if (Via(result)->kind != PTF_deref) {
			    PtrGenerate(&result);
			}
		    }
		    if (!(resulttype = isFunctionType(isPointerType(GetTreeTP(result))))) {
			SemanticError("Call of non-function");
		    }
		    if (NextIs(')')) {
			if (GetTPKind(ftype) == TRC_ANSIfunction) {
			    SemanticError("This function requires arguments");
			}
			/*
			 * The above error assumes that ANSIfunctions are the
			 * only ones which have arguments.  An ANSI function
			 * with no arguments is internally represented as a
			 * NOARGS function. We do not care if there is a call
			 * to an old style function which is declared with
			 * args, with no args.
			 */
			result = BuildTreeNode(PTF_function_call, result, NULL, NULL);
			Via(result)->data.TP = functype;
			SetTreeTP(result, resulttype);
			SetTreeLValue(result, 0);
		    } else {
			tmp = Do_argument_expr_list();
			/*
			 * The arg expr list is a list of expressions. If
			 * there were none, tmp is NULL.  If there was more
			 * than one, then Via(tmp)->kind ==
			 * PTF_argument_list.  Otherwise, tmp is simply the
			 * single expr which was an arg.  If there was more
			 * than one, then the list is in reverse order.  The
			 * first arg is in Via(tmp)->a and the leftover args
			 * are in yet another list (same properties apply) in
			 * Via(tmp)->b.  So, if there were 2 args, then
			 * Via(tmp)->a is the second arg, and Via(tmp)->b is
			 * the second.
			 */
			if (!NextIs(')')) {
			    SyntaxError("Missing right parenthesis");
			}
			if (tmp) {
			    if (GetTPKind(ftype) == TRC_NOARGSfunction) {
				SemanticError("This function has no parameters");
			    } else if ((GetTPKind(ftype) == TRC_ANSIELLIPSISfunction) ||
				   (GetTPKind(ftype) == TRC_ANSIfunction)) {
				/*
				 * Here we are type checking the arguments in
				 * tmp (the argument expr list for the
				 * function call) against the proto in prot.
				 * tmp is a parse tree and prot is a symbol
				 * table.
				 */
				ProtoTypeCheck(prot, &tmp, (GetTPKind(ftype) == TRC_ANSIELLIPSISfunction));
				result = BuildTreeNode(PTF_function_call, result, tmp, NULL);
				Via(result)->data.TP = functype;
				SetTreeTP(result, resulttype);
				SetTreeLValue(result, 0);
			    } else {
				done = 0;
				iscount = 0;
				ndx = 1;
				carg = &tmp;
				while (!done) {
				    prevcarg = *carg;
				    if (Via((*carg))->kind == PTF_argument_list) {
					PtrGenerate(&(Via((*carg))->a));
					StandardIntPromote(&(Via((*carg))->a));
					StandardFloatPromote(&(Via((*carg))->a));
				    } else {
					/* Last arg. */
					PtrGenerate(carg);
					StandardIntPromote(carg);
					StandardFloatPromote(carg);
					done = 1;
				    }
				    carg = &((Via(prevcarg))->b);	/* Next arg. */
				    ndx++;
				    iscount++;
				}
				result = BuildTreeNode(PTF_function_call, result, tmp, NULL);
				Via(result)->data.TP = functype;
				SetTreeTP(result, resulttype);
				SetTreeLValue(result, 0);
			    }
			} else {
			    SyntaxError("Expecting argument list for func call");
			}
		    }
		}
		break;
	    case '.':
		/*
		 * TYPERULE The type of result must be a struct or union.
		 */
		if (LeftUndeclared) {
		    SemanticError2("Undeclared identifier : ", idfound);
		    ident = TableAdd(GlobalSymbolTable, (idfound));
		    SetSymTP(ident, BuildTypeRecord(0, TRC_int, SGN_signed));
		    Via(ident)->numbers.CountUses++;
		    LeftUndeclared = 0;
		    SetTreeTP(result, GetSymTP(ident));
		    SetTreeLValue(result, 0);
		    Via(result)->data.thesymbol = ident;
		}
		if (!isStructUnionType(GetTreeTP(result))) {
		    TypeError("Struct or union type required here for .");
		}
		if (NextIs(IDENTIFIER)) {
		    /*
		     * TYPERULE The identifier must be the name of a member
		     * of the struct/union in result. The type of the
		     * resulting postfix expr is the type of the named
		     * member.  It is an lvalue if the member is not an array
		     * type.
		     */
		    SYMVia_t                        memb;
		    if ((memb = isMemberOf(GetTreeTP(result), LastToken)) != 0) {
			ParseTreeVia_t                  old;
			old = result;
			result = BuildTreeNode(PTF_struct_member, result, NULL, NULL);
			Via(result)->data.thesymbol = memb;
			SetTreeTP(result, GetSymTP(memb));
			if (!isArrayType(GetSymTP(memb))) {
			    if (Via(old)->kind == PTF_identifier) {
				if (GetTPQual(Via(Via(old)->data.thesymbol)->TP) == SCC_const) {
				    SetTreeLValue(result, 0);
				} else {
				    SetTreeLValue(result, 1);
				}
			    } else {
				SetTreeLValue(result, 1);
			    }
			} else {
			    SetTreeLValue(result, 0);
			}
		    } else {
			SemanticError("This is not a member of the given struct/union (.)");
		    }
		} else {
		    SyntaxError("Expecting identifier for field name");
		}
		break;
	    case PTR_OP:
		/*
		 * TYPERULE The type of result must be ptr to struct or
		 * union.
		 */
		if (LeftUndeclared) {
		    SemanticError2("Undeclared identifier : ", idfound);
		    ident = TableAdd(GlobalSymbolTable, (idfound));
		    SetSymTP(ident, BuildTypeRecord(0, TRC_int, SGN_signed));
		    Via(ident)->numbers.CountUses++;
		    LeftUndeclared = 0;
		    SetTreeTP(result, GetSymTP(ident));
		    SetTreeLValue(result, 0);
		    Via(result)->data.thesymbol = ident;
		}
		if (!isStructUnionType(isPointerType(GetTreeTP(result)))) {
		    TypeError("Struct or union type required for ->");
		}
		if (NextIs(IDENTIFIER)) {
		    /*
		     * TYPERULE The identifier must be the name of a member
		     * of the struct/union in result. The type of the
		     * resulting postfix expr is the type of the named
		     * member.  It is an lvalue if the member is not an array
		     * type.
		     */
		    SYMVia_t                        memb;
		    if ((memb = isMemberOf(isPointerType(GetTreeTP(result)), LastToken)) != 0) {
			ParseTreeVia_t                  old;
			old = result;
			result = BuildTreeNode(PTF_struct_indirect_member, result, NULL, NULL);
			Via(result)->data.thesymbol = memb;
			SetTreeTP(result, GetSymTP(memb));
			if (!isArrayType(GetSymTP(memb))) {
			    SetTreeLValue(result, 1);
			} else {
			    SetTreeLValue(result, 0);
			}
		    } else {
			SemanticError("This is not a member of the given struct/union (->)");
		    }
		} else {
		    SyntaxError("Expecting identifier for field name");
		}
		break;
	    case INC_OP:{
		    /*
		     * TYPERULE The operand must have arithmetic type, and it
		     * must be an lvalue.  The resulting type is the same,
		     * but it is not an lvalue.
		     */
		    ParseTreeVia_t                  tmptree;
		    if (LeftUndeclared) {
			SemanticError2("Undeclared identifier : ", idfound);
			ident = TableAdd(GlobalSymbolTable, (idfound));
			SetSymTP(ident, BuildTypeRecord(0, TRC_int, SGN_signed));
			Via(ident)->numbers.CountUses++;
			LeftUndeclared = 0;
			SetTreeTP(result, GetSymTP(ident));
			SetTreeLValue(result, 0);
			Via(result)->data.thesymbol = ident;
		    }
		    if (!isBooleanType(GetTreeTP(result))) {
			TypeError("++ requires arithmetic or pointer type");
		    }
		    if (!GetTreeLValue(result)) {
			SemanticError("Operand of ++ must be an lvalue");
		    }
		    tmptree = result;
		    result = BuildTreeNode(PTF_postincrement, tmptree, NULL, NULL);
		    SetTreeLValue(result, 0);
		    SetTreeTP(result, GetTreeTP(tmptree));
		}
		break;
	    case DEC_OP:{
		    /*
		     * TYPERULE The operand must have arithmetic type, and it
		     * must be an lvalue.  The resulting type is the same,
		     * but it is not an lvalue.
		     */
		    ParseTreeVia_t                  tmptree;
		    if (LeftUndeclared) {
			SemanticError2("Undeclared identifier : ", idfound);
			ident = TableAdd(GlobalSymbolTable, (idfound));
			SetSymTP(ident, BuildTypeRecord(0, TRC_int, SGN_signed));
			Via(ident)->numbers.CountUses++;
			LeftUndeclared = 0;
			SetTreeTP(result, GetSymTP(ident));
			SetTreeLValue(result, 0);
			Via(result)->data.thesymbol = ident;
		    }
		    if (!isBooleanType(GetTreeTP(result))) {
			TypeError("-- requires arithmetic or pointer type");
		    }
		    if (!GetTreeLValue(result)) {
			SemanticError("Operand of -- must be an lvalue");
		    }
		    tmptree = result;
		    result = BuildTreeNode(PTF_postdecrement, tmptree, NULL, NULL);
		    SetTreeLValue(result, 0);
		    SetTreeTP(result, GetTreeTP(tmptree));
		}
		break;
	    default:
		donepostfixing = 1;
		UnFetchToken();
	    }
	}
    }
    if (result) {
	if (!GetTreeTP(result)) {
	    if (InPreprocIf) {
		SetTreeTP(result, BuildTypeRecord(0, TRC_int, SGN_signed));
		SetTreeLValue(result, 0);
		Via(result)->data.thesymbol = NULL;
	    } else {
		SemanticError2("Undeclared identifier : ", idfound);
		ident = TableAdd(GlobalSymbolTable, (idfound));
		SetSymTP(ident, BuildTypeRecord(0, TRC_int, SGN_signed));
		Via(ident)->numbers.CountUses++;
		LeftUndeclared = 0;
		SetTreeTP(result, GetSymTP(ident));
		SetTreeLValue(result, 0);
		Via(result)->data.thesymbol = ident;
	    }
	}
    }
    return result;
}

ParseTreeVia_t
Do_argument_expr_list(void)
{
    /*
     * argument_expr_list : assignment_expr | argument_expr_list ','
     * assignment_expr ;
     */

    ParseTreeVia_t                  result;
    int                             donelisting;
    ParseTreeVia_t                  tmp;
    int                             count;
    count = 0;
    result = Do_assignment_expr();
    if (result) {
	donelisting = 0;
	while (!donelisting) {
	    if (NextIs(',')) {
		tmp = Do_assignment_expr();
		if (tmp) {
		    result = BuildTreeNode(PTF_argument_list, tmp, result, NULL);
		} else {
		    SyntaxError("Expecting argument expr in function call");
		}
	    } else {
		donelisting = 1;
	    }
	}
    }
    /*
     * QQQQ This routine does not have type rules.  It is simply returning a
     * list of expressions.  As such, the data structures here may not be the
     * best choice, and this routine should probably move farther down in the
     * list.
     */
    return result;
}

ParseTreeVia_t
Do_unary_expr(void)
{
    /*
     * unary_expr : postfix_expr | INC_OP unary_expr | DEC_OP unary_expr |
     * unary_operator cast_expr | SIZEOF unary_expr | SIZEOF '(' type_name
     * ')' ;
     */

    TypeRecordVia_t                 typerec;
    ParseTreeVia_t                  result = NULL;
    ParseTreeVia_t                  tmp;
    /*
     * TYPERULE For ++ and --, the operand must be an lvalue of arithmetic
     * type, and the result is NOT an lvalue.
     */
    if (NextIs(INC_OP)) {
	tmp = Do_unary_expr();
	if (tmp) {
	    if (!isBooleanType(GetTreeTP(tmp))) {
		TypeError("++ requires arithmetic or pointer type");
	    }
	    if (!GetTreeLValue(tmp)) {
		SemanticError("Operand of ++ must be an lvalue");
	    }
	    result = BuildTreeNode(PTF_preincrement, tmp, NULL, NULL);
	    SetTreeTP(result, GetTreeTP(tmp));
	    SetTreeLValue(result, 0);
	} else {
	    SyntaxError("Expected unary expr after ++");
	}
    } else if (NextIs(DEC_OP)) {
	tmp = Do_unary_expr();
	if (tmp) {
	    if (!isBooleanType(GetTreeTP(tmp))) {
		TypeError("-- requires arithmetic or pointer type");
	    }
	    if (!GetTreeLValue(tmp)) {
		SemanticError("Operand of -- must be an lvalue");
	    }
	    result = BuildTreeNode(PTF_predecrement, tmp, NULL, NULL);
	    SetTreeTP(result, GetTreeTP(tmp));
	    SetTreeLValue(result, 0);
	} else {
	    SyntaxError("Expected unary expr after --");
	}
    } else if (NextIs(DEFINED)) {
	int                             needrightparen;
	char                            PStr[128];
	char                            c;
	int                             ndx;
	/*
	 * defined is a token which is not always a token.  The isKeyword
	 * routine checks the global flag (preprocif) and only returns true
	 * iff this flag is true, when it find the token 'defined'. The
	 * preproc directives set this flag true when parsing a #if or #elif
	 * expression.  The flag is set false immediately after, because the
	 * defined() macro is not valid except during preprocessing.
	 */
	needrightparen = 0;
	if (NextIs('(')) {
	    needrightparen = 1;
	}
	c = NonSpaceCharacter();
	ndx = 0;
	if (isFirstIDChar(c)) {
	    PStr[ndx++] = c;
	    while (isAnyIDChar(c = TokenCharacter())) {
		PStr[ndx++] = c;
	    }
	    PStr[ndx] = 0;
	    /*
	     * Now PStr contains the name of the identifer being tested.
	     */
	    PutBackChar(c);
	} else {
	    PutBackChar(c);
	}
	if (ndx) {
	    result = BuildTreeNode(PTF_intconstant, NULL, NULL, NULL);
	    SetTreeTP(result, BuildTypeRecord(0, TRC_int, SGN_signed));
	    SetTreeLValue(result, 0);
	    if (isDefined(PStr)) {
		Via(result)->data.number = 1;
	    } else {
		Via(result)->data.number = 0;
	    }
	} else {
	    SyntaxError("Expected identifier after preproc 'defined'");
	}
	if (needrightparen) {
	    if (!NextIs(')')) {
		SyntaxError("Missing right parenthesis");
	    }
	}
    } else if (NextIs(SIZEOF)) {
	if (NextIs('(')) {
	    typerec = Do_type_name();
	    if (typerec) {
		if (isIncompleteType(typerec)) {
		    TypeError("Incomplete type for sizeof");
		}
		if (isBitFieldType(typerec)) {
		    TypeError("Cannot take sizeof a bit field");
		}
		result = BuildTreeNode(PTF_sizeof, NULL, NULL, NULL);
		Via(result)->data.TP = typerec;
		SetTreeTP(result, BuildTypeRecord(0, TRC_long, SGN_unsigned));
		SetTreeLValue(result, 0);
	    } else {
		/*
		 * Here, we will allow sizeof(expr) even though I'm not sure
		 * if it is ANSI.
		 */
		tmp = Do_conditional_expr();
		if (tmp) {
		    if (isBitFieldType(GetTreeTP(tmp))) {
			TypeError("Cannot take sizeof a bit field");
		    }
		    if (isIncompleteType(GetTreeTP(tmp))) {
			TypeError("Incomplete type for sizeof");
		    }
		    result = BuildTreeNode(PTF_sizeof, NULL, NULL, NULL);
		    Via(result)->data.TP = GetTreeTP(tmp);
		    SetTreeTP(result, BuildTypeRecord(0, TRC_long, SGN_unsigned));
		    SetTreeLValue(result, 0);
		} else {
		    SyntaxError("Expected unary expr or typename in sizeof");
		}
	    }
	    if (!NextIs(')')) {
		SyntaxError("Missing right parenthesis");
	    }
	} else {
	    tmp = Do_unary_expr();
	    if (tmp) {
		if (isBitFieldType(GetTreeTP(tmp))) {
		    TypeError("Cannot take sizeof a bit field");
		}
		if (isIncompleteType(GetTreeTP(tmp))) {
		    TypeError("Incomplete type for sizeof");
		}
		result = BuildTreeNode(PTF_sizeof, NULL, NULL, NULL);
		Via(result)->data.TP = GetTreeTP(tmp);
		SetTreeTP(result, BuildTypeRecord(0, TRC_long, SGN_unsigned));
		SetTreeLValue(result, 0);
	    } else {
		SyntaxError("Expected unary expr after sizeof");
	    }
	}
    } else {
	Codigo_t                        unaryop;
	unaryop = Do_unary_operator();
	if (unaryop) {
	    tmp = Do_cast_expr();
	    if (tmp) {
		TypeRecordVia_t                 tmptype;
		switch (unaryop) {
		case '&':
		    if (Via(tmp)->kind == PTF_identifier) {
			if (Via(Via(tmp)->data.thesymbol)->storage_class == SCC_register) {
			    SemanticError("& (address-of) is illegal for register variables");
			}
		    }
		    if (isBitFieldType(GetTreeTP(tmp))) {
			SemanticError("& (address-of) is illegal for bit fields");
		    }
		    result = BuildTreeNode(PTF_address_of, tmp, NULL, NULL);
		    SetTreeLValue(result, 0);
		    SetTreeTP(result, BuildTypeRecord(GetTreeTP(tmp), TRC_pointer, SGN_unknown));
		    break;
		case '*':
		    /* TYPERULE This must be a pointer. */
		    PtrGenerate(&tmp);
		    if (!(tmptype = isPointerType(GetTreeTP(tmp)))) {
			TypeError("Dereferencing requires expr of pointer type");
		    }
		    result = BuildTreeNode(PTF_deref, tmp, NULL, NULL);
		    SetTreeTP(result, tmptype);
		    if (isArrayType(GetTreeTP(tmp))) {
			SetTreeLValue(result, 0);
		    } else {
			SetTreeLValue(result, 1);
		    }
		    break;
		case '+':
		    /* TYPERULE ??? */
		    if (!isArithmeticType(GetTreeTP(tmp))) {
			TypeError("Unary plus requires arithmetic type");
		    }
		    result = BuildTreeNode(PTF_unary_plus, tmp, NULL, NULL);
		    SetTreeTP(result, GetTreeTP(tmp));
		    SetTreeLValue(result, 0);
		    break;
		case '-':
		    /* TYPERULE ??? */
		    if (!isArithmeticType(GetTreeTP(tmp))) {
			TypeError("Unary minus requires arithmetic type");
		    }
		    result = BuildTreeNode(PTF_unary_minus, tmp, NULL, NULL);
		    SetTreeTP(result, GetTreeTP(tmp));
		    SetTreeLValue(result, 0);
		    break;
		case '~':
		    /* TYPERULE ??? */
		    if (!isIntegralType(GetTreeTP(tmp))) {
			TypeError("~ requires integral type");
		    }
		    result = BuildTreeNode(PTF_bitwise_neg, tmp, NULL, NULL);
		    SetTreeTP(result, GetTreeTP(tmp));
		    SetTreeLValue(result, 0);
		    break;
		case '!':
		    /* TYPERULE ??? */
		    if (!isBooleanType(GetTreeTP(tmp))) {
			TypeError("! requires arithmetic or pointer type");
		    }
		    result = BuildTreeNode(PTF_logical_neg, tmp, NULL, NULL);
		    SetTreeTP(result, BuildTypeRecord(0, TRC_int, SGN_signed));
		    SetTreeLValue(result, 0);
		    break;
		default:
		    VeryBadParseError("Illegal token code for unary operator");
		    break;
		}
	    } else {
		SyntaxError("Expected expr after unary operator");
	    }
	} else {
	    result = Do_postfix_expr();
	}
    }
    return result;
}

Codigo_t
Do_unary_operator(void)
{
    /*
     * unary_operator : '&' | '*' | '+' | '-' | '~' | '!' ;
     */

    if (NextIs('&')) {
	return '&';
    } else if (NextIs('*')) {
	return '*';
    } else if (NextIs('+')) {
	return '+';
    } else if (NextIs('-')) {
	return '-';
    } else if (NextIs('~')) {
	return '~';
    } else if (NextIs('!')) {
	return '!';
    } else {
	return 0;
    }
}
