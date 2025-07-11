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
 * This file implements operations on ParseTrees.
 * 
 */

/*
 * The following routines handle pointer generation, integer promotion, and
 * coersion for assignment, math operations, comparisons, and function calls.
 */

#include "conditcomp.h"
#include <stdio.h>
#include <string.h>

#include "structs.h"

#pragma segment ParseTrees

#include "TypeRecord.h"
#include "ParseTree.h"

void
PtrGenerate(ParseTreeVia_t * tree)
/*
 * This routine converts arrays to pointers and functions to pointers to
 * functions.
 */
{
    TypeRecordVia_t                 thetype;
    thetype = StripTypedef(GetTreeTP(*tree));
    switch (GetTPKind(thetype)) {
    case TRC_array:
	(*tree) = BuildTreeNode(PTF_gen_addrof, *tree, NULL, NULL);
	SetTreeTP(*tree, BuildTypeRecord(isArrayType(thetype),
					 TRC_pointer, SGN_unknown));
	Via(*tree)->LValue = 0;
	break;
    case TRC_ANSIfunction:
    case TRC_ANSIELLIPSISfunction:
    case TRC_OLDfunction:
    case TRC_NOARGSfunction:
	(*tree) = BuildTreeNode(PTF_gen_addrof, *tree, NULL, NULL);
	SetTreeTP(*tree, BuildTypeRecord(thetype,
					 TRC_pointer, SGN_unknown));
	Via((*tree))->LValue = 0;
	break;
    }
}

void
FuncPtrGenerate(ParseTreeVia_t * tree)
{
    TypeRecordVia_t                 thetype;
    thetype = StripTypedef(GetTreeTP(*tree));
    switch (GetTPKind(thetype)) {
    case TRC_ANSIfunction:
    case TRC_ANSIELLIPSISfunction:
    case TRC_OLDfunction:
    case TRC_NOARGSfunction:
	(*tree) = BuildTreeNode(PTF_gen_addrof, *tree, NULL, NULL);
	SetTreeTP(*tree, BuildTypeRecord(thetype,
					 TRC_pointer, SGN_unknown));
	Via((*tree))->LValue = 0;
	break;
    }
}

ParseTreeVia_t
TypeConvert(ParseTreeVia_t expr, TypeRecordVia_t newtype)
{
    /*
     * We need to have a separate node here for a typechange, because some
     * type changes and conversions require specific reformatting. For
     * example, a float cast to an int requires significant changes.  A
     * pointer cast to an int probably requires that the value be moved from
     * an A register to a D register.
     */
    assert(!isFunctionType(GetTreeTP(expr)));
    if (isFloatingType(newtype)) {
	if (isIntegralType(GetTreeTP(expr))) {
	    if (!(GetTPSize(GetTreeTP(expr)) == 4 && GetTPSign(GetTreeTP(expr)) == SGN_signed)) {
		expr = TypeConvert(expr, BuildTypeRecord(0, TRC_long, SGN_signed));
	    }
	}
    } else if (isIntegralType(newtype)) {
	if (isFloatingType(GetTreeTP(expr))) {
	    if (!(GetTPSize(newtype) == 4 && GetTPSign(newtype) == SGN_signed)) {
		expr = TypeConvert(expr, BuildTypeRecord(0, TRC_long, SGN_signed));
	    }
	}
    }
    expr = BuildTreeNode(PTF_typechange, expr, NULL, NULL);
    SetTreeTP(expr, newtype);
    return expr;
}

void
StandardIntPromote(ParseTreeVia_t * a)
{
    /*
     * TODO According to ANSI, this routine should convert unsigned char to
     * int, and unsigned short to int iff sizeof(int) > sizeof(short)
     */

    if (GetTPKind(GetTreeTP(*a)) == TRC_char) {
	(*a) = TypeConvert((*a), BuildTypeRecord(0, TRC_int, SGN_signed));
    }
    if (GetTPKind(GetTreeTP(*a)) == TRC_short) {
	(*a) = TypeConvert((*a), BuildTypeRecord(0, TRC_int, SGN_signed));
    }
    if (GetTPKind(GetTreeTP(*a)) == TRC_bitfield) {
	(*a) = TypeConvert((*a), BuildTypeRecord(0, TRC_int, SGN_signed));
    }
    if (GetTPKind(GetTreeTP(*a)) == TRC_enum) {
	SetTreeTP(*a, CopyTypeRecord(GetTreeTP(*a)));
	SetTPKind(GetTreeTP(*a), TRC_int);
    }
}

void
StandardFloatPromote(ParseTreeVia_t * a)
{
    if (GetTPKind(GetTreeTP(*a)) == TRC_float) {
	(*a) = TypeConvert((*a), BuildTypeRecord(0, TRC_double, SGN_unknown));
    }
}

TypeRecordVia_t
Coerce(ParseTreeVia_t * a, ParseTreeVia_t * b, int mixpointers)
{
    /*
     * This routine will coerce the two operands into compatible types and
     * return the type of the result.
     */

    if (!((isArithmeticType(GetTreeTP(*b))) && (isArithmeticType(GetTreeTP(*a))))) {
	if (isStructUnionType(GetTreeTP(*a))) {
	    TypeError("Illegal usage of struct/union");
	}
	if (isStructUnionType(GetTreeTP(*b))) {
	    TypeError("Illegal usage of struct/union");
	}
	PtrGenerate(a);
	PtrGenerate(b);

	if (isFloatingType(GetTreeTP(*a))) {
	    if (isPointerType(GetTreeTP(*b))) {
		TypeError("Floats and pointers may not be mixed in expressions");
	    }
	} else if (isFloatingType(GetTreeTP(*b))) {
	    if (isPointerType(GetTreeTP(*a))) {
		TypeError("Floats and pointers may not be mixed in expressions");
	    }
	}
	if ((isPointerType(GetTreeTP(*a))) && (isPointerType(GetTreeTP(*b)))) {
	    if (!SameType(GetTreeTP(*a), GetTreeTP(*b))) {
		if (!mixpointers) {
		    TypeError("Pointers of unlike types may not be mixed.");
		}
	    }
	}
	if ((!isPointerType(GetTreeTP(*a))) && (isPointerType(GetTreeTP(*b)))) {
	    StandardIntPromote(a);
	    return GetTreeTP(*b);
	} else {
	    StandardIntPromote(b);
	    return GetTreeTP(*a);
	}

    } else {
	if (GetTPKind(GetTreeTP(*a)) == TRC_longdouble) {
	    if (GetTPKind(GetTreeTP(*b)) != TRC_longdouble) {
		(*b) = TypeConvert((*b), GetTreeTP(*a));
		return GetTreeTP(*a);
	    }
	} else if (GetTPKind(GetTreeTP(*b)) == TRC_longdouble) {
	    if (GetTPKind(GetTreeTP(*a)) != TRC_longdouble) {
		(*a) = TypeConvert((*a), GetTreeTP(*b));
		return GetTreeTP(*a);
	    }
	}
	if (GetTPKind(GetTreeTP(*a)) == TRC_double) {
	    if (GetTPKind(GetTreeTP(*b)) != TRC_double) {
		(*b) = TypeConvert((*b), GetTreeTP(*a));
		return GetTreeTP(*a);
	    }
	} else if (GetTPKind(GetTreeTP(*b)) == TRC_double) {
	    if (GetTPKind(GetTreeTP(*a)) != TRC_double) {
		(*a) = TypeConvert((*a), GetTreeTP(*b));
		return GetTreeTP(*a);
	    }
	}
	if (GetTPKind(GetTreeTP(*a)) == TRC_float) {
	    if (GetTPKind(GetTreeTP(*b)) != TRC_float) {
		(*b) = TypeConvert((*b), GetTreeTP(*a));
		return GetTreeTP(*a);
	    }
	} else if (GetTPKind(GetTreeTP(*b)) == TRC_float) {
	    if (GetTPKind(GetTreeTP(*a)) != TRC_float) {
		(*a) = TypeConvert((*a), GetTreeTP(*b));
		return GetTreeTP(*a);
	    }
	}
	StandardIntPromote(a);
	StandardIntPromote(b);

	if ((GetTPKind(GetTreeTP(*a)) == TRC_long) && (GetTPSign(GetTreeTP(*a)) == SGN_unsigned)) {
	    if (!SameType(GetTreeTP(*a), GetTreeTP(*b))) {
		(*b) = TypeConvert((*b), GetTreeTP(*a));
		return GetTreeTP(*a);
	    }
	} else if ((GetTPKind(GetTreeTP(*b)) == TRC_long) && (GetTPSign(GetTreeTP(*b)) == SGN_unsigned)) {
	    if (!SameType(GetTreeTP(*b), GetTreeTP(*a))) {
		(*a) = TypeConvert((*a), GetTreeTP(*b));
		return GetTreeTP(*a);
	    }
	}
	if ((GetTPKind(GetTreeTP(*a)) == TRC_long)) {
	    if ((GetTPKind(GetTreeTP(*b)) == TRC_int) && (GetTPSign(GetTreeTP(*b)) == SGN_unsigned)) {
		if (GetTPSize(GetTreeTP(*b)) < GetTPSize(GetTreeTP(*a))) {
		    (*b) = TypeConvert((*b), GetTreeTP(*a));
		    return GetTreeTP(*a);
		} else {
		    TypeRecordVia_t                 temp;
		    temp = BuildTypeRecord(0, TRC_long, SGN_unsigned);
		    (*b) = TypeConvert((*b), temp);
		    (*a) = TypeConvert((*a), temp);
		    return GetTreeTP(*a);
		}
	    }
	} else if ((GetTPKind(GetTreeTP(*b)) == TRC_long)) {
	    if ((GetTPKind(GetTreeTP(*a)) == TRC_int) && (GetTPSign(GetTreeTP(*a)) == SGN_unsigned)) {
		if (GetTPSize(GetTreeTP(*a)) < GetTPSize(GetTreeTP(*b))) {
		    (*a) = TypeConvert((*a), GetTreeTP(*b));
		    return GetTreeTP(*a);
		} else {
		    TypeRecordVia_t                 temp;
		    temp = BuildTypeRecord(0, TRC_long, SGN_unsigned);
		    (*a) = TypeConvert((*a), temp);
		    (*b) = TypeConvert((*b), temp);
		    return GetTreeTP(*a);
		}
	    }
	}
	if (GetTPKind(GetTreeTP(*a)) == TRC_long) {
	    if (GetTPKind(GetTreeTP(*b)) != TRC_long) {
		(*b) = TypeConvert((*b), GetTreeTP(*a));
		return GetTreeTP(*a);
	    }
	} else if (GetTPKind(GetTreeTP(*b)) == TRC_long) {
	    if (GetTPKind(GetTreeTP(*a)) != TRC_long) {
		(*a) = TypeConvert((*a), GetTreeTP(*b));
		return GetTreeTP(*a);
	    }
	}
	if (GetTPSign(GetTreeTP(*a)) == SGN_unsigned) {
	    if (GetTPSign(GetTreeTP(*b)) != SGN_unsigned) {
		(*b) = TypeConvert((*b), GetTreeTP(*a));
		return GetTreeTP(*a);
	    }
	} else if (GetTPSign(GetTreeTP(*b)) == SGN_unsigned) {
	    if (GetTPSign(GetTreeTP(*a)) != SGN_unsigned) {
		(*a) = TypeConvert((*a), GetTreeTP(*b));
		return GetTreeTP(*a);
	    }
	}
	/* Then, both are of type int. */
	return GetTreeTP(*a);
    }
    TypeError("Incompatible types in expression.");
    return 0;
}

void
RelateCoerce(ParseTreeVia_t * a, ParseTreeVia_t * b)
{
    (void) Coerce(a, b, 0);
    if (isPointerType(GetTreeTP(*a)) && (isIntegralType(GetTreeTP(*b)))) {
	UserWarning(WARN_comparepointerint);
    }
    if (isPointerType(GetTreeTP(*b)) && (isIntegralType(GetTreeTP(*a)))) {
	UserWarning(WARN_comparepointerint);
    }
}

TypeRecordVia_t
AssignCoerce(ParseTreeVia_t * a, ParseTreeVia_t * b)
{
    /*
     * This routine is used during assignment, to ensure that the two
     * expressions are of compatible type.
     */
    TypeRecordVia_t                 result;
    result = 0;

    /* TODO Redo according to ANSI. */
    PtrGenerate(b);

    if (isArithmeticType(GetTreeTP(*a))) {
	if (!SameType(GetTreeTP(*a), GetTreeTP(*b))) {
	    if (isArithmeticType(GetTreeTP(*b))) {
		(*b) = TypeConvert((*b), GetTreeTP(*a));
	    }
	}
    } else if (isArithmeticType(GetTreeTP(*b))) {
	if (!SameType(GetTreeTP(*a), GetTreeTP(*b))) {
	    if (isArithmeticType(GetTreeTP(*a))) {
		(*b) = TypeConvert((*b), GetTreeTP(*a));
	    }
	}
    }
    if ((isStructUnionType(GetTreeTP(*a))) || (isStructUnionType(GetTreeTP(*b)))) {
	if (!SameType(GetTreeTP(*a), GetTreeTP(*b))) {
	    /* These two types must be identical. */
	    TypeError("Invalid struct/union assignment");
	}
    }
    if (isFloatingType(GetTreeTP(*a))) {
	if (isPointerType(GetTreeTP(*b))) {
	    TypeError("Floats may not be assigned to pointers.");
	}
    } else if (isPointerType(GetTreeTP(*a))) {
	if (!SameType(GetTreeTP(*a), GetTreeTP(*b))) {
	    if (isFloatingType(GetTreeTP(*b))) {
		TypeError("Floats may not be assigned to pointers.");
	    }
	    /*
	     * The warning below should not be issued if the type of b is
	     * either (void *) or b is an integer constant with value of 0
	     * (in other words, NULL).
	     */
	    if (!((Via((*b))->kind == PTF_intconstant) && (Via((*b))->data.number == 0))) {
		if (!isVoidPointerType(GetTreeTP(*b))) {
		    /*
		     * The SameType call above - these pointer types must be
		     * identical.
		     */
		    UserWarning(WARN_pointernonequivassign);
		}
		(*b) = TypeConvert((*b), GetTreeTP(*a));
	    } else {
		(*b) = TypeConvert((*b), GetTreeTP(*a));
	    }
	}
    }
    result = GetTreeTP(*a);	/* What purpose does the return value of this
				 * routine serve ? */

    return result;
}

void
ReturnCoerce(TypeRecordVia_t param, ParseTreeVia_t * expr)
{
    /*
     * This routine handles type checking and coercion for function values
     * returns.
     */

    PtrGenerate(expr);

    if (isIntegralType(param)) {
	if (isArithmeticType(GetTreeTP(*expr))) {
	    (*expr) = TypeConvert((*expr), param);
	} else {
	    TypeError("Non arithmetic types may not be returned into integral types");
	}
    }
    if (isFloatingType(param)) {
	if (isArithmeticType(GetTreeTP(*expr))) {
	    (*expr) = TypeConvert((*expr), param);
	} else {
	    TypeError("Non arithmetic types may not be returned into floating types");
	}
    } else if (isPointerType(param)) {
	if (!SameType(param, GetTreeTP(*expr))) {
	    if (isFloatingType(GetTreeTP(*expr))) {
		TypeError("Floats may not be returned into pointers");
	    }
	    /*
	     * The warning below should not be issued if the type of expr is
	     * either (void *) or expr is an integer constant with value of 0
	     * (in other words, NULL).
	     */
	    if (!((Via((*expr))->kind == PTF_intconstant) && (Via((*expr))->data.number == 0))) {
		if ((!isVoidPointerType(param)) && (!isVoidPointerType(GetTreeTP(*expr)))) {
		    /*
		     * The SameType call above - these pointer types must be
		     * identical.
		     */
		    UserWarning(WARN_pointernonequivreturn);
		} else {
		    (*expr) = TypeConvert((*expr), param);
		}
	    } else {
		(*expr) = TypeConvert((*expr), param);
	    }
	}
    }
}

void
CallCoerce(TypeRecordVia_t param, ParseTreeVia_t * expr)
{
    /*
     * This routine handles type checking and coercion for ANSI function
     * calls.
     */

    PtrGenerate(expr);

    if (isIntegralType(param)) {
	if (isArithmeticType(GetTreeTP(*expr))) {
	    (*expr) = TypeConvert((*expr), param);
	} else {
	    TypeError("Non arithmetic types may not be passed into integral types");
	}
    }
    if (isFloatingType(param)) {
	if (isArithmeticType(GetTreeTP(*expr))) {
	    (*expr) = TypeConvert((*expr), param);
	} else {
	    TypeError("Non arithmetic types may not be passed into floating types");
	}
    } else if (isPointerType(param)) {
	if (!SameType(param, GetTreeTP(*expr))) {
	    if (isFloatingType(GetTreeTP(*expr))) {
		TypeError("Floats may not be passed into pointers");
	    }
	    /*
	     * The warning below should not be issued if the type of expr is
	     * either (void *) or expr is an integer constant with value of 0
	     * (in other words, NULL).
	     */
	    if (!((Via((*expr))->kind == PTF_intconstant) && (Via((*expr))->data.number == 0))) {
		if ((!isVoidPointerType(param)) && (!isVoidPointerType(GetTreeTP(*expr)))) {
		    /*
		     * The SameType call above - these pointer types must be
		     * identical.
		     */
		    UserWarning(WARN_pointernonequivarg);
		}
		(*expr) = TypeConvert((*expr), param);
	    } else {
		(*expr) = TypeConvert((*expr), param);
	    }
	}
    }
    if (!SameType(param, GetTreeTP(*expr))) {
	TypeError("Type mismatch on argument passing");
    }
}

ParseTreeVia_t
RawTreeNode(char *id)
{
    register ParseTreeVia_t         raw;
#ifdef Undefined
    if (id)
	raw = Ealloc(sizeof(ParseTree_t) + strlen(id));
    else
#endif
	raw = Ealloc(sizeof(ParseTree_t));
    if (raw) {
	Via(raw)->kind = 0;
	Via(raw)->LValue = 0;
	Via(raw)->data.number = 0;
	SetTreeTP(raw, 0);
	Via(raw)->a = NULL;
	Via(raw)->b = NULL;
	Via(raw)->c = NULL;
	Via(raw)->for_Scopes.d = NULL;
	if (id)
	    Via(raw)->data.identifier = PutString(ParserStrings, id);
    }
    return raw;
}

ParseTreeVia_t
BuildTreeNode(enum ParseTreeFormat kind, ParseTreeVia_t a, ParseTreeVia_t b, ParseTreeVia_t c)
/*
 * Principal routine for building expression trees.  Note that it accepts
 * only subtrees a, b and c, while there is also a field d. This is because
 * only for stmts used the d field, and I didn't want to waste so much
 * time/space in argument passing all those NULLs.  The routine which handles
 * for statements sets this field itelf.
 */
{
    ParseTreeVia_t                  node;
    node = RawTreeNode(NULL);
    Via(node)->kind = kind;
    Via(node)->a = a;
    Via(node)->b = b;
    Via(node)->c = c;
    return node;
}

ParseTreeVia_t
BuildIDTreeNode(enum ParseTreeFormat kind, ParseTreeVia_t a, ParseTreeVia_t b, ParseTreeVia_t c, char *id)
/*
 * Principal routine for building expression trees.  Note that it accepts
 * only subtrees a, b and c, while there is also a field d. This is because
 * only for stmts used the d field, and I didn't want to waste so much
 * time/space in argument passing all those NULLs.  The routine which handles
 * for statements sets this field itelf.
 */
{
    ParseTreeVia_t                  node;
    node = RawTreeNode(id);
    Via(node)->kind = kind;
    Via(node)->a = a;
    Via(node)->b = b;
    Via(node)->c = c;
    return node;
}

ParseTreeVia_t
GetTreeFour(ParseTreeVia_t tree)
{
    assert(tree);
    return Via(tree)->for_Scopes.d;
}

ScopesVia_t
GetTreeScopes(ParseTreeVia_t tree)
{
    assert(tree);
    return Via(tree)->for_Scopes.Scopes;
}

void
SetTreeFour(ParseTreeVia_t tree, ParseTreeVia_t d)
{
    assert(tree);
    Via(tree)->for_Scopes.d = d;
}

void
SetTreeScopes(ParseTreeVia_t tree, ScopesVia_t s)
{
    assert(tree);
    Via(tree)->for_Scopes.Scopes = s;
}

void
SetTreeTP(ParseTreeVia_t tree, TypeRecordVia_t TP)
{
    assert(tree);
    Via(tree)->for_Scopes.TP = TP;
}

TypeRecordVia_t
GetTreeTP(ParseTreeVia_t tree)
{
    assert(tree);
    return Via(tree)->for_Scopes.TP;
}

short
GetTreeLValue(ParseTreeVia_t tree)
{
    assert(tree);
    return Via(tree)->LValue;
}

void
SetTreeLValue(ParseTreeVia_t tree, short v)
{
    assert(tree);
    Via(tree)->LValue = v;
}

void
FreeTree(ParseTreeVia_t root)
{
    /*
     * We need to consider disposal of Locals, Scopes, TP, a, b, c, and d
     */
    if (root) {
	FreeTree(Via(root)->a);
	FreeTree(Via(root)->b);
	FreeTree(Via(root)->c);
	if (Via(root)->kind == PTF_for_stmt)
	    FreeTree(GetTreeFour(root));
	else if (Via(root)->kind == PTF_compound_stmt)
	    if (GetTreeScopes(root)) {
		FreeSymbolList(Via(GetTreeScopes(root))->Symbols, 1);
		FreeSymbolList(Via(GetTreeScopes(root))->Tags, 1);
		FreeSymbolList(Via(GetTreeScopes(root))->Labels, 1);
		FreeSymbolList(Via(GetTreeScopes(root))->Enums, 1);
		Efree(GetTreeScopes(root));
	    }
	Efree(root);
    }
}
