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
 * This file contains functions for various optimizations and transformations.
 * 
 * 
 */


#include "conditcomp.h"
#include <stdio.h>
#include <string.h>
#include "structs.h"


#pragma segment Peephole


ParseTreeVia_t
BuildConstantNode(FoldValue_t * val)
{
    ParseTreeVia_t                  result;
    if (!val->isK) {
	return NULL;
    }
    /*
     * Are we going to have problems here with signed and unsigned numbers ?
     */
    if (val->isint) {
	if (val->isunsigned) {
	    FreeTree(val->init);
	    result = BuildTreeNode(PTF_intconstant, NULL, NULL, NULL);
	    SetTreeTP(result, BuildTypeRecord(0, TRC_long, SGN_unsigned));
	    Via(result)->LValue = 0;
	    Via(result)->data.number = val->uintval;
	} else {
	    FreeTree(val->init);
	    result = BuildTreeNode(PTF_intconstant, NULL, NULL, NULL);
	    SetTreeTP(result, BuildTypeRecord(0, TRC_long, SGN_signed));
	    Via(result)->LValue = 0;
	    Via(result)->data.number = val->intval;
	}
    } else if (val->issymb) {
	result = val->init;
    } else {
	FloatLitVia_t                   lit;
	FreeTree(val->init);
	result = BuildTreeNode(PTF_floatconstant, NULL, NULL, NULL);
	SetTreeTP(result, BuildTypeRecord(0, TRC_longdouble, SGN_unknown));
	Via(result)->LValue = 0;
	lit = AddFloatLit(val->realval);
	Via(result)->data.FLit = lit;
    }
    return result;
}

void
ConstExprValue(ParseTreeVia_t root, FoldValue_t * resp)
{
    FoldValue_t                     result, notK;
    FoldValue_t                     t1, t2;

    notK.isK = 0;
    result = notK;
    result.isint = 1;
    result.isstring = result.issymb = result.isoffset = 0;
    result.init = root;
    result.isunsigned = 0;
    result.intval = 0;
    result.thesymbol = NULL;
    result.uintval = 0;
    result.realval = 0;

#define GetBoth()  \
      ConstExprValue(Via(root)->a, &t1); \
      ConstExprValue(Via(root)->b, &t2); \
      if ((!t1.isK) || (!t2.isK)) { \
	result = notK; break; \
      } result.isK = 1;


    if (root) {
	switch (Via(root)->kind) {
	case PTF_array_subscript:
	    ConstExprValue(Via(root)->a, &t1);
	    ConstExprValue(Via(root)->b, &t2);
	    if (t1.thesymbol && t2.isK) {
		result.isK = 1;
		result.isint = 0;
		result.isoffset = 1;
		result.intval = t2.intval * GetTPSize(GetTreeTP(root));
		result.thesymbol = t1.thesymbol;
	    }
	    break;
	case PTF_function_call:
	case PTF_postincrement:
	case PTF_postdecrement:
	case PTF_argument_list:
	case PTF_preincrement:
	case PTF_predecrement:
	case PTF_deref:
	    break;
	case PTF_gen_addrof:
	case PTF_address_of:
	    ConstExprValue(Via(root)->a, &result);
	    break;
	case PTF_unary_plus:
	    ConstExprValue(Via(root)->a, &result);
	    break;
	case PTF_unary_minus:
	    ConstExprValue(Via(root)->a, &t1);
	    if (!t1.isK) {
		result = notK;
		break;
	    }
	    result.isK = 1;
	    result.isunsigned = t1.isunsigned;
	    result.isint = t1.isint;
	    if (t1.isint) {
		if (t1.isunsigned) {
		    result.uintval = -t1.uintval;
		} else {
		    result.intval = -t1.intval;
		}
	    } else {
		result.realval = -t1.realval;
	    }
	    break;
	case PTF_bitwise_neg:
	    ConstExprValue(Via(root)->a, &t1);
	    if (!t1.isK) {
		result = notK;
		break;
	    }
	    result.isK = 1;
	    result.isint = 1;
	    result.isunsigned = t1.isunsigned;
	    result.intval = ~t1.intval;
	    result.uintval = ~t1.uintval;
	    result.realval = t1.intval;
	    break;
	case PTF_logical_neg:
	    ConstExprValue(Via(root)->a, &t1);
	    if (!t1.isK) {
		result = notK;
		break;
	    }
	    result.isK = 1;
	    result.isint = 1;
	    result.isunsigned = 0;
	    if (t1.isint) {
		if (t1.isunsigned) {
		    result.intval = !t1.uintval;
		} else {
		    result.intval = !t1.intval;
		}
	    } else {
		result.intval = !t1.realval;
	    }
	    break;
	case PTF_multiply:
	    GetBoth();
	    if (t1.isint) {
		if (t2.isint) {
		    result.isint = 1;
		    if (t1.isunsigned) {
			if (t2.isunsigned) {
			    result.isunsigned = 1;
			    result.uintval = t1.uintval * t2.uintval;
			} else {
			    result.isunsigned = 1;
			    result.uintval = t1.uintval * t2.intval;
			}
		    } else {
			if (t2.isunsigned) {
			    result.isunsigned = 1;
			    result.uintval = t1.intval * t2.uintval;
			} else {
			    result.isunsigned = 0;
			    result.intval = t1.intval * t2.intval;
			}
		    }
		} else {
		    result.isint = 0;
		    result.isunsigned = 0;
		    if (t1.isunsigned) {
			result.realval = t1.uintval * t2.realval;
		    } else {
			result.realval = t1.intval * t2.realval;
		    }
		}
	    } else {
		result.isint = 0;
		result.isunsigned = 0;
		if (t2.isint) {
		    if (t2.isunsigned) {
			result.realval = t1.realval * t2.uintval;
		    } else {
			result.realval = t1.realval * t2.intval;
		    }
		} else {
		    result.realval = t1.realval * t2.realval;
		}
	    }
	    break;
	case PTF_divide:
	    GetBoth();
	    if (t1.isint) {
		if (t2.isint) {
		    result.isint = 1;
		    if (t1.isunsigned) {
			if (t2.isunsigned) {
			    result.isunsigned = 1;
			    result.uintval = t1.uintval / t2.uintval;
			} else {
			    result.isunsigned = 1;
			    result.uintval = t1.uintval / t2.intval;
			}
		    } else {
			if (t2.isunsigned) {
			    result.isunsigned = 1;
			    result.uintval = t1.intval / t2.uintval;
			} else {
			    result.isunsigned = 0;
			    result.intval = t1.intval / t2.intval;
			}
		    }
		} else {
		    result.isint = 0;
		    result.isunsigned = 0;
		    if (t1.isunsigned) {
			result.realval = t1.uintval / t2.realval;
		    } else {
			result.realval = t1.intval / t2.realval;
		    }
		}
	    } else {
		result.isint = 0;
		result.isunsigned = 0;
		if (t2.isint) {
		    if (t2.isunsigned) {
			result.realval = t1.realval / t2.uintval;
		    } else {
			result.realval = t1.realval / t2.intval;
		    }
		} else {
		    result.realval = t1.realval / t2.realval;
		}
	    }
	    break;
	case PTF_modulo:
	    GetBoth();
	    result.intval = t1.intval % t2.intval;
	    result.realval = result.intval;
	    if (t1.isint && t2.isint) {
		result.isint = 1;
	    } else {
		result.isint = 0;
	    }
	    break;
	case PTF_typechange:
	    /*
	     * Quite possibly something should be done here.  Many
	     * typechanges are actually constants.
	     */
	    break;
	case PTF_add:
	    GetBoth();
	    result.intval = t1.intval + t2.intval;
	    result.realval = t1.realval + t2.realval;
	    if (t1.isint && t2.isint) {
		result.isint = 1;
	    } else {
		result.isint = 0;
	    }
	    break;
	case PTF_subtract:
	    GetBoth();
	    result.intval = t1.intval - t2.intval;
	    result.realval = t1.realval - t2.realval;
	    if (t1.isint && t2.isint) {
		result.isint = 1;
	    } else {
		result.isint = 0;
	    }
	    break;
	case PTF_shift_left:
	    GetBoth();
	    result.intval = t1.intval << t2.intval;
	    result.realval = result.intval;
	    if (t1.isint && t2.isint) {
		result.isint = 1;
	    } else {
		result.isint = 0;
	    }
	    break;
	case PTF_shift_right:
	    GetBoth();
	    result.intval = t1.intval >> t2.intval;
	    result.realval = result.intval;
	    if (t1.isint && t2.isint) {
		result.isint = 1;
	    } else {
		result.isint = 0;
	    }
	    break;
	case PTF_lessthan:
	    /* All these relationals should probably be handled too. */
	    break;
	case PTF_greaterthan:
	    break;
	case PTF_lessthaneq:
	    break;
	case PTF_greaterthaneq:
	    break;
	case PTF_equal:
	    break;
	case PTF_notequal:
	    break;
	case PTF_bitwise_and:
	    GetBoth();
	    result.intval = t1.intval & t2.intval;
	    result.realval = result.intval;
	    if (t1.isint && t2.isint) {
		result.isint = 1;
	    } else {
		result.isint = 0;
	    }
	    break;
	case PTF_bitwise_xor:
	    GetBoth();
	    result.intval = t1.intval ^ t2.intval;
	    result.realval = result.intval;
	    if (t1.isint && t2.isint) {
		result.isint = 1;
	    } else {
		result.isint = 0;
	    }
	    break;
	case PTF_bitwise_or:
	    GetBoth();
	    result.intval = t1.intval | t2.intval;
	    result.realval = result.intval;
	    if (t1.isint && t2.isint) {
		result.isint = 1;
	    } else {
		result.isint = 0;
	    }
	    break;
	case PTF_logical_and:
	    GetBoth();
	    result.intval = t1.intval && t2.intval;
	    result.isint = 1;
	    break;
	case PTF_logical_or:
	    GetBoth();
	    result.intval = t1.intval || t2.intval;
	    result.isint = 1;
	    break;
	case PTF_initializer_list:
	    /*
	     * With an initializer list, we return the const value of the
	     * first item in the list.  The list is stored a=head, b=tail.
	     */
	    ConstExprValue(Via(root)->a, &result);
	    break;
	case PTF_ternary:
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
	case PTF_commas:
	case PTF_multi_initializer:
	case PTF_exprstmt:
	case PTF_emptystmt:
	case PTF_switchcase_stmt:
	case PTF_switchdefault_stmt:
	case PTF_compound_stmt:
	case PTF_stmt_list:
	case PTF_ifthenelse_stmt:
	case PTF_switch_stmt:
	case PTF_while_stmt:
	case PTF_dowhile_stmt:
	case PTF_for_stmt:
	case PTF_continue_stmt:
	case PTF_break_stmt:
	case PTF_return_stmt:
	    break;
	case PTF_identifier:
	    result.issymb = 1;
	    result.thesymbol = Via(root)->data.thesymbol;
	    result.isint = 0;
	    result.isK = 0;
	    break;
	case PTF_enumconstant:
	    result.isK = 1;
	    result.isint = 1;
	    result.intval = Via(Via(root)->data.thesymbol)->numbers.EnumVal;
	    break;
	case PTF_sizeof:
	    result.isK = 1;
	    result.isint = 1;
	    result.intval = GetTPSize(Via(root)->data.TP);
	    break;
	case PTF_intconstant:
	    result.isK = 1;
	    result.isint = 1;
	    if (isUnsignedType(GetTreeTP(root))) {
		result.isunsigned = 0;
		result.uintval = Via(root)->data.number;
	    } else {
		result.intval = Via(root)->data.number;
	    }
	    break;
	case PTF_floatconstant:
	    result.isK = 1;
	    result.isint = 0;
	    result.realval = Via(Via(root)->data.FLit)->val.num;
	    result.intval = result.realval;
	    break;
	case PTF_string_literal:
	    result.isK = 1;
	    result.isint = 0;
	    result.isstring = 1;
	    break;
	case PTF_struct_member:
	case PTF_struct_indirect_member:
	case PTF_labelled_stmt:
	case PTF_goto_stmt:
	default:
	    break;
	}
    }
    *resp = result;
}

/*
 * The real problems with a constant folding routines are in getting the
 * various types to fold together correctly.  Currently, I don't even think
 * ecc handles unsigned constants correctly.
 */

ParseTreeVia_t
Constify(ParseTreeVia_t tree)
{
    FoldValue_t                     res;
    FoldValue_t                     tK1;
    ParseTreeVia_t                  newK;
    if (tree) {
	switch (Via(tree)->kind) {
	case PTF_intconstant:
	case PTF_floatconstant:
	case PTF_enumconstant:
	case PTF_string_literal:
	    return tree;
	    break;
	default:
	    ConstExprValue(tree, &res);
	    if (res.isK && !res.issymb) {
		newK = BuildConstantNode(&res);
		return newK;
	    }
	    break;
	}
	switch (Via(tree)->kind) {
	case PTF_multiply:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    return tree;
	    break;
	case PTF_divide:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    return tree;
	    break;
	case PTF_modulo:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    return tree;
	    break;
	case PTF_add:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    return tree;
	    break;
	case PTF_subtract:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    return tree;
	    break;
	case PTF_shift_left:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    return tree;
	    break;
	case PTF_shift_right:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    return tree;
	    break;
	case PTF_lessthan:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    return tree;
	    break;
	case PTF_greaterthan:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    return tree;
	    break;
	case PTF_lessthaneq:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    return tree;
	    break;
	case PTF_greaterthaneq:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    return tree;
	    break;
	case PTF_equal:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    return tree;
	    break;
	case PTF_notequal:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    return tree;
	    break;
	case PTF_bitwise_and:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    return tree;
	    break;
	case PTF_bitwise_xor:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    return tree;
	    break;
	case PTF_bitwise_or:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    return tree;
	    break;
	case PTF_logical_and:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    return tree;
	    break;
	case PTF_logical_or:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    return tree;
	    break;
	case PTF_ternary:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    Via(tree)->c = Constify(Via(tree)->c);
	    return tree;
	    break;
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
	    Via(tree)->b = Constify(Via(tree)->b);
	    return tree;
	    break;
	case PTF_stmt_list:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    return tree;
	    break;
	case PTF_ifthenelse_stmt:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    Via(tree)->c = Constify(Via(tree)->c);
	    ConstExprValue(Via(tree)->a, &tK1);
	    if (tK1.isK) {
		if (tK1.isint) {
		    if (tK1.intval) {
			return Via(tree)->b;
		    } else {
			if (Via(tree)->c) {
			    return Via(tree)->c;
			} else {
			    FreeTree(tree);
			    return BuildTreeNode(PTF_NOP, NULL, NULL, NULL);
			}
		    }
		}
	    }
	    return tree;
	    break;
	case PTF_exprstmt:
	case PTF_compound_stmt:
	    Via(tree)->a = Constify(Via(tree)->a);
	    return tree;
	    break;
	case PTF_while_stmt:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    ConstExprValue(Via(tree)->a, &tK1);
	    if (tK1.isK) {
		if (tK1.isint) {
		    if (!tK1.intval) {
			return BuildTreeNode(PTF_NOP, NULL, NULL, NULL);
		    }
		}
	    }
	    return tree;
	    break;
	case PTF_dowhile_stmt:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    return tree;
	    break;
	case PTF_for_stmt:
	    Via(tree)->a = Constify(Via(tree)->a);
	    Via(tree)->b = Constify(Via(tree)->b);
	    Via(tree)->c = Constify(Via(tree)->c);
	    SetTreeFour(tree, Constify(GetTreeFour(tree)));
	    return tree;
	    break;
	case PTF_return_stmt:
	    Via(tree)->a = Constify(Via(tree)->a);
	    return tree;
	    break;
	default:
	    break;
	}
    }
    return tree;
}

/* Optimizations... */

int
isRegDirect(LocAMVia_t loc)
{
    if (!loc)
	return 0;
    if (GetLocAM(loc) == M68am_DReg) {
	return 1;
    } else if (GetLocAM(loc) == M68am_ARegDirect) {
	return 1;
    } else {
	return 0;
    }
}

InstVia_t
DeleteInst(InstVia_t inst)
/*
 * Removes an instruction from the list, and returns the instruction just
 * before it.
 */
{
#ifdef DoDelete
    InstVia_t                       result;
    if (Via(inst)->prev) {
	Via(Via(inst)->prev)->next = Via(inst)->next;
    }
    if (Via(inst)->next) {
	Via(Via(inst)->next)->prev = Via(inst)->prev;
    }
    result = Via(inst)->prev;
    Efree(inst);
    return result;
#else
	Via(inst)->OP = M68op_DELETED;
    return inst;
#endif
}

int
isLABEL(InstVia_t a)
{
    if (!a)
	return 0;
    switch (Via(a)->OP) {
    case M68op_DATALABEL:
    case M68op_CODELABEL:
    case M68op_STRINGLABEL:
    case M68op_LCOMM:
    case M68op_COMM:
    case M68op_LABEL:
	return 1;
    default:
	return 0;
    }
}

int
isBRANCH(InstVia_t a)
{
    if (!a)
	return 0;
    switch (Via(a)->OP) {
    case M68op_BRA:
    case M68op_BCC:
    case M68op_BCS:
    case M68op_BEQ:
    case M68op_BGE:
    case M68op_BGT:
    case M68op_BHI:
    case M68op_BLE:
    case M68op_BLS:
    case M68op_BLT:
    case M68op_BMI:
    case M68op_BNE:
    case M68op_BPL:
    case M68op_BVC:
    case M68op_BVS:
	return 1;
    default:
	return 0;
    }
}

int
EquivAddress(InstVia_t a, InstVia_t b)
{
    InstVia_t                       c;
    /* Return true if the two instructions are the same address */
    if (a == b)
	return 1;
    /* check forward from a */
    if (isLABEL(a)) {
	c = Via(a)->next;
	while (c) {
	    if (c == b)
		return 1;
	    if (isLABEL(c)) {
		c = Via(c)->next;
	    } else {
		c = NULL;
	    }
	}
	c = Via(a)->prev;
	while (c) {
	    if (c == b)
		return 1;
	    if (isLABEL(c)) {
		c = Via(c)->prev;
	    } else {
		c = NULL;
	    }
	}
    } else if (Via(b)->OP == M68op_LABEL) {
	c = Via(b)->next;
	while (c) {
	    if (c == a)
		return 1;
	    if (isLABEL(c)) {
		c = Via(c)->next;
	    } else {
		c = NULL;
	    }
	}
	c = Via(b)->prev;
	while (c) {
	    if (c == a)
		return 1;
	    if (isLABEL(c)) {
		c = Via(c)->prev;
	    } else {
		c = NULL;
	    }
	}
    } else
	return 0;
}

void
Optimize68(InstListVia_t Codes)
{
    InstVia_t                       curinst;
    curinst = Via(Codes)->head;
    while (curinst) {
	int                             deleted;
	deleted = 0;
	if (isBRANCH(curinst)) {
	    /*
	     * We SHOULD check for ANY branch here, and insert a nop or
	     * something.
	     */
	    if (Via(curinst)->next) {
		if (Via(Via(curinst)->next)->OP == M68op_LABEL) {
		    if (EquivAddress(Via(curinst)->next, Via(GetLocLabel(Via(curinst)->left))->M68kDef.where)) {
			curinst = DeleteInst(curinst);
			curinst = Via(curinst)->prev;
			deleted = 1;
		    }
		}
		/*
		 * The inst after the branch should probably be deleted. It
		 * is dead code.
		 */
	    }
	} else if (Via(curinst)->OP == M68op_MOVE) {
	    /*
	     * There are many other optimizations which can be done on move
	     * instructions.
	     */
	    if (GetLocAM(Via(curinst)->left) == M68am_Immediate) {
		if (GetLocAM(Via(curinst)->right) == M68am_DReg) {
		    if ((GetLocConstant(Via(curinst)->left) <= 127) && (GetLocConstant(Via(curinst)->left)
								 >= -128)) {
			Via(curinst)->OP = M68op_MOVEQ;
			Via(curinst)->SZ = M68sz_none;
		    }
		}
	    } else if (SameLocation(Via(curinst)->left, Via(curinst)->right)) {
		curinst = DeleteInst(curinst);
		deleted = 1;
	    } else if (Via(curinst)->prev) {
		if (Via(Via(curinst)->prev)->OP == M68op_MOVE) {
		    if (Via(Via(curinst)->prev)->SZ == Via(curinst)->SZ)
			if (SameLocation(Via(Via(curinst)->prev)->right, Via(curinst)->left) &&
			    SameLocation(Via(Via(curinst)->prev)->left, Via(curinst)->right)) {
			    curinst = DeleteInst(curinst);
			    deleted = 1;
			}
		}
	    }
	} else if (Via(curinst)->OP == M68op_ADD) {
	    if (GetLocAM(Via(curinst)->left) == M68am_Immediate) {
		if ((GetLocConstant(Via(curinst)->left) <= 8) && (GetLocConstant(Via(curinst)->left)
								  >= 1)) {
		    Via(curinst)->OP = M68op_ADDQ;
		}
	    }
	} else if (Via(curinst)->OP == M68op_SUB) {
	    if (GetLocAM(Via(curinst)->left) == M68am_Immediate) {
		if ((GetLocConstant(Via(curinst)->left) <= 8) && (GetLocConstant(Via(curinst)->left)
								  >= 1)) {
		    Via(curinst)->OP = M68op_SUBQ;
		}
	    }
	}
	if (!deleted) {
	    curinst = Via(curinst)->next;
	}
    }
}
