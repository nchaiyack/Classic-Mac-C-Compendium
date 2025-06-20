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
 * This file contains routines for 68k code generation.  These routines handle
 * expressions with side effects.
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

#pragma segment GenSide


LocAMVia_t
GenAssign(ParseTreeVia_t left, ParseTreeVia_t right, InstListVia_t Codes)
/* General routine for generating an assignment node. */
{
    /*
     * An example of a correct code generation routine.  Never assume the
     * sizes of the types of the arguments, without checking.  Call FreeTemp
     * on all Loc records not returned from the routine.
     */

    LocAMVia_t                      rightoperand;
    LocAMVia_t                      result;
    rightoperand = GenExpression(right, Codes);
    result = GenExpression(left, Codes);

    GenDeConflictize(&rightoperand, &result, Codes);

    if (GetLocAM(result) == M68am_ARegDisplaceFIELD) {
	unsigned long                   bitmask;
	LocAMVia_t                      prev;
	prev = TempDataReg(Codes);
	Genmove(M68sz_long, result, prev, Codes);
	/* Construct a mask, and AND immed */
	bitmask = ~MakeMask(result);
	GenInst(M68op_AND, GetLocSZ(result),
		BuildImmediate(bitmask, M68sz_long), prev, Codes);

	rightoperand = DictateTempDReg(rightoperand, Codes);
	ShiftLeft(rightoperand, GetLocFieldBits(result).StartBit, Codes);
	GenInst(M68op_OR, GetLocSZ(result), rightoperand, prev, Codes);
	Genmove(M68sz_long, prev, result, Codes);
	FreeTemp(prev);
    } else {
	Genmove(GetLocSZ(result), rightoperand, result, Codes);
    }
    FreeTemp(rightoperand);
    return result;
}

LabSYMVia_t                     LibLabelULMODT;
LabSYMVia_t                     LibLabelLMODT;
LabSYMVia_t                     LibLabelULMULT;
LabSYMVia_t                     LibLabelULDIVT;
LabSYMVia_t                     LibLabelLDIVT;
LabSYMVia_t                     LibLabelXTOI;

void
SetupLibLabels(void)
{
    LibLabelULMODT = MakeSysLabel("ULMODT");
    LibLabelLMODT = MakeSysLabel("LMODT");
    LibLabelULMULT = MakeSysLabel("ULMULT");
    LibLabelULDIVT = MakeSysLabel("ULDIVT");
    LibLabelLDIVT = MakeSysLabel("LDIVT");
    LibLabelXTOI = MakeSysLabel("_XTOI");
}

LocAMVia_t
GenModAssign(ParseTreeVia_t left, ParseTreeVia_t right, InstListVia_t Codes)
{
    /* Integer only. */
    LocAMVia_t                      leftop;
    LocAMVia_t                      rightop;
    LocAMVia_t                      lefttemp;
    lefttemp = leftop = GenExpression(left, Codes);
    rightop = GenExpression(right, Codes);
    if (isUnsignedType(GetTreeTP(left))) {
	if (GetTPSize(GetTreeTP(left)) > 2) {
	    /*
	     * All these various methods for handling long math should be
	     * better structured, and copied to all other places where this
	     * occurs.
	     */
	    if (!gProject->itsOptions->useMC68020) {
		leftop = GenLibraryCall(leftop, rightop,
					LibLabelULMODT, Codes);
		Genmove(M68_TypeSize(GetTreeTP(left)), leftop, lefttemp, Codes);
	    } else {
		/*
		 * Currently we do long mod with libraries, even in 68020
		 * mode.  There must be a better way...
		 */
		leftop = GenLibraryCall(leftop, rightop,
					LibLabelULMODT, Codes);
		Genmove(M68_TypeSize(GetTreeTP(left)), leftop, lefttemp, Codes);
	    }
	} else {
	    leftop = DictateTempDReg(leftop, Codes);
	    rightop = DictateTempDReg(rightop, Codes);
	    GenInst(M68op_DIVU, M68sz_none, rightop, leftop, Codes);
	    GenInst(M68op_SWAP, M68sz_none, leftop, NULL, Codes);
	    GenInst(M68op_EXT, M68sz_long, leftop, NULL, Codes);
	    Genmove(M68_TypeSize(GetTreeTP(left)), leftop, lefttemp, Codes);
	}
    } else {
	if (GetTPSize(GetTreeTP(left)) > 2) {
	    if (!gProject->itsOptions->useMC68020) {
		leftop = GenLibraryCall(leftop, rightop,
					LibLabelLMODT, Codes);
		Genmove(M68_TypeSize(GetTreeTP(left)), leftop, lefttemp, Codes);
	    } else {
		/*
		 * We currently do long mod with library even in 68020 mode.
		 */
		leftop = GenLibraryCall(leftop, rightop,
					LibLabelLMODT, Codes);
		Genmove(M68_TypeSize(GetTreeTP(left)), leftop, lefttemp, Codes);
	    }
	} else {
	    leftop = DictateTempDReg(leftop, Codes);
	    rightop = DictateTempDReg(rightop, Codes);
	    GenInst(M68op_DIVS, M68sz_none, rightop, leftop, Codes);
	    GenInst(M68op_SWAP, M68sz_none, leftop, NULL, Codes);
	    GenInst(M68op_EXT, M68sz_long, leftop, NULL, Codes);
	    Genmove(M68_TypeSize(GetTreeTP(left)), leftop, lefttemp, Codes);
	}
    }
    FreeTemp(rightop);
    FreeTemp(leftop);
    return lefttemp;
}

LocAMVia_t
GenMulDivAssign(Opcode_t OP, ParseTreeVia_t left, ParseTreeVia_t right, InstListVia_t Codes)
{
    LocAMVia_t                      leftop;
    LocAMVia_t                      rightop;
    LocAMVia_t                      lefttemp;
    lefttemp = leftop = GenExpression(left, Codes);
    rightop = GenExpression(right, Codes);
    if (isFloatingType(GetTreeTP(left))) {
	leftop = DictateTempFloat(leftop, Codes);
	if (gProject->itsOptions->useMC68881) {
	    if (isShortFloatingType(GetTreeTP(left))) {
		switch (OP) {
		case M68op_MULS:
		    GenInst(M68op_FSGLMUL, M68sz_single, rightop, leftop, Codes);
		    break;
		case M68op_DIVS:
		    GenInst(M68op_FSGLDIV, M68sz_single, rightop, leftop, Codes);
		    break;
		}
	    } else {
		switch (OP) {
		case M68op_MULS:
		    GenInst(M68op_FMUL, GetLocSZ(rightop), rightop, leftop, Codes);
		    break;
		case M68op_DIVS:
		    GenInst(M68op_FDIV, GetLocSZ(rightop), rightop, leftop, Codes);
		    break;
		}
	    }
	    Genmove(GetLocSZ(lefttemp), leftop, lefttemp, Codes);
	} else {
	    LocAMVia_t                      ftemp;
	    ftemp = TempFloatReg(Codes);
	    if (isShortFloatingType(GetTreeTP(left))) {
		GenSANECall(FFSGL + FOZ2X, leftop, ftemp, Codes);
		switch (OP) {
		case M68op_MULS:
		    GenSANECall(FFSGL + FOMUL, rightop, ftemp, Codes);
		    break;
		case M68op_DIVS:
		    GenSANECall(FFSGL + FODIV, rightop, ftemp, Codes);
		    break;
		}
		GenSANECall(FFSGL + FOX2Z, ftemp, leftop, Codes);
	    } else if (isExtendedFloatingType(GetTreeTP(left))) {
		GenSANECall(FFEXT + FOZ2X, leftop, ftemp, Codes);
		switch (OP) {
		case M68op_MULS:
		    GenSANECall(FFEXT + FOMUL, rightop, ftemp, Codes);
		    break;
		case M68op_DIVS:
		    GenSANECall(FFEXT + FODIV, rightop, ftemp, Codes);
		    break;
		}
		GenSANECall(FFEXT + FOX2Z, ftemp, leftop, Codes);
	    } else {
		GenSANECall(FFDBL + FOZ2X, leftop, ftemp, Codes);
		switch (OP) {
		case M68op_MULS:
		    GenSANECall(FFDBL + FOMUL, rightop, ftemp, Codes);
		    break;
		case M68op_DIVS:
		    GenSANECall(FFDBL + FODIV, rightop, ftemp, Codes);
		    break;
		}
		GenSANECall(FFDBL + FOX2Z, ftemp, leftop, Codes);
	    }
	    GenSANECall(FFEXT + FOZ2X, ftemp, lefttemp, Codes);
	}
    } else if (isBitFieldType(GetTreeTP(left))) {
	unsigned long                   bitmask;
	unsigned long                   mask2;
	LocAMVia_t                      prev;
	LocAMVia_t                      val2;
	prev = TempDataReg(Codes);
	Genmove(M68sz_long, leftop, prev, Codes);
	/* Construct a mask, and AND immed */
	bitmask = MakeMask(leftop);
	mask2 = ~bitmask;
	GenInst(M68op_AND, GetLocSZ(leftop),
		BuildImmediate(bitmask, M68sz_long), prev, Codes);

	ShiftRight(prev, GetLocFieldBits(leftop).StartBit, Codes);
	if (GetTPSize(GetTreeTP(right)) > 2) {
	    if (!gProject->itsOptions->useMC68020) {
		switch (OP) {
		case M68op_MULS:
		    prev = GenLibraryCall(prev, rightop,
					  LibLabelULMULT, Codes);
		    break;
		case M68op_DIVS:
		    prev = GenLibraryCall(prev, rightop,
					  LibLabelULDIVT, Codes);
		    break;
		}
	    } else {
		switch (OP) {
		case M68op_MULS:
		    GenInst(M68op_MULU, M68sz_long, rightop, prev, Codes);
		    break;
		case M68op_DIVS:
		    GenInst(M68op_DIVU, M68sz_long, rightop, prev, Codes);
		    break;
		}
	    }
	}
	ShiftLeft(prev, GetLocFieldBits(leftop).StartBit, Codes);
	val2 = TempDataReg(Codes);
	Genmove(M68sz_long, leftop, val2, Codes);
	/* Construct a mask, and AND immed */
	GenInst(M68op_AND, GetLocSZ(leftop),
		BuildImmediate(mask2, M68sz_long), val2, Codes);
	GenInst(M68op_OR, GetLocSZ(leftop), prev, val2, Codes);
	Genmove(M68sz_long, val2, leftop, Codes);
	FreeTemp(prev);
	FreeTemp(val2);
    } else if (isUnsignedType(GetTreeTP(left))) {
	if (GetTPSize(GetTreeTP(left)) > 2) {
	    if (!gProject->itsOptions->useMC68020) {
		switch (OP) {
		case M68op_MULS:
		    leftop = GenLibraryCall(leftop, rightop,
					    LibLabelULMULT, Codes);
		    break;
		case M68op_DIVS:
		    leftop = GenLibraryCall(leftop, rightop,
					    LibLabelULDIVT, Codes);
		    break;
		}
		Genmove(M68_TypeSize(GetTreeTP(left)), leftop, lefttemp, Codes);
	    } else {
		leftop = DictateTempDReg(leftop, Codes);
		rightop = DictateTempDReg(rightop, Codes);
		switch (OP) {
		case M68op_MULS:
		    GenInst(M68op_MULU, M68sz_long, rightop, leftop, Codes);
		    break;
		case M68op_DIVS:
		    GenInst(M68op_DIVU, M68sz_long, rightop, leftop, Codes);
		    break;
		}
		Genmove(M68_TypeSize(GetTreeTP(left)), leftop, lefttemp, Codes);
	    }
	} else {
	    leftop = DictateTempDReg(leftop, Codes);
	    rightop = DictateTempDReg(rightop, Codes);
	    switch (OP) {
	    case M68op_MULS:
		GenInst(M68op_MULU, M68sz_none, rightop, leftop, Codes);
		break;
	    case M68op_DIVS:
		GenInst(M68op_DIVU, M68sz_none, rightop, leftop, Codes);
		break;
	    }
	    Genmove(M68_TypeSize(GetTreeTP(left)), leftop, lefttemp, Codes);
	}
    } else {
	if (GetTPSize(GetTreeTP(left)) > 2) {
	    if (!gProject->itsOptions->useMC68020) {
		switch (OP) {
		case M68op_MULS:
		    leftop = GenLibraryCall(leftop, rightop,
					    LibLabelULMULT, Codes);
		    break;
		case M68op_DIVS:
		    leftop = GenLibraryCall(leftop, rightop,
					    LibLabelLDIVT, Codes);
		    break;
		}
		Genmove(M68_TypeSize(GetTreeTP(left)), leftop, lefttemp, Codes);
	    } else {
		leftop = DictateTempDReg(leftop, Codes);
		rightop = DictateTempDReg(rightop, Codes);
		GenInst(OP, M68sz_long, rightop, leftop, Codes);
		Genmove(M68_TypeSize(GetTreeTP(left)), leftop, lefttemp, Codes);
	    }
	} else {
	    leftop = DictateTempDReg(leftop, Codes);
	    rightop = DictateTempDReg(rightop, Codes);
	    GenInst(OP, M68sz_none, rightop, leftop, Codes);
	    Genmove(M68_TypeSize(GetTreeTP(left)), leftop, lefttemp, Codes);
	}
    }
    FreeTemp(rightop);
    FreeTemp(leftop);
    return lefttemp;
}

LocAMVia_t
GenOPAssignDREG(Opcode_t OP, ParseTreeVia_t left, ParseTreeVia_t right, InstListVia_t Codes)
{
    LocAMVia_t                      leftop;
    LocAMVia_t                      lefttemp;
    LocAMVia_t                      rightop;
    /*
     * This routine, used only for shifts and xor, should accomodate signed
     * and unsigned in the same manner. Of course, floats are illegal here.
     * Also, long and short operands should be handled the same here too. So,
     * there is only one case.
     */
    lefttemp = leftop = GenExpression(left, Codes);
    rightop = GenExpression(right, Codes);
    if (GetLocAM(leftop) == M68am_ARegDisplaceFIELD) {
	unsigned long                   bitmask;
	unsigned long                   mask2;
	LocAMVia_t                      prev;
	LocAMVia_t                      val2;
	prev = TempDataReg(Codes);
	Genmove(M68sz_long, leftop, prev, Codes);
	/* Construct a mask, and AND immed */
	bitmask = MakeMask(leftop);
	mask2 = ~bitmask;
	GenInst(M68op_AND, GetLocSZ(leftop),
		BuildImmediate(bitmask, M68sz_long), prev, Codes);

	ShiftRight(prev, GetLocFieldBits(leftop).StartBit, Codes);
	GenInst(OP, GetLocSZ(leftop), rightop, prev, Codes);
	ShiftLeft(prev, GetLocFieldBits(leftop).StartBit, Codes);
	val2 = TempDataReg(Codes);
	Genmove(M68sz_long, leftop, val2, Codes);
	GenInst(M68op_AND, GetLocSZ(leftop),
		BuildImmediate(mask2, M68sz_long), val2, Codes);
	GenInst(M68op_OR, GetLocSZ(leftop), prev, val2, Codes);
	Genmove(M68sz_long, val2, leftop, Codes);
	FreeTemp(prev);
	FreeTemp(val2);
    } else {
	leftop = DictateTempDReg(leftop, Codes);
	rightop = DictateTempDReg(rightop, Codes);
	GenInst(OP, M68_TypeSize(GetTreeTP(left)), rightop, leftop, Codes);
	Genmove(M68_TypeSize(GetTreeTP(left)), leftop, lefttemp, Codes);
	FreeTemp(leftop);
    }
    FreeTemp(rightop);
    return lefttemp;
}

LocAMVia_t
GenOPAssign(Opcode_t OP, ParseTreeVia_t left, ParseTreeVia_t right, InstListVia_t Codes)
{
    LocAMVia_t                      leftop;
    LocAMVia_t                      rightop;
    if (isFloatingType(GetTreeTP(left))) {
	leftop = GenExpression(left, Codes);
	rightop = GenExpression(right, Codes);
	rightop = DictateTempFloat(rightop, Codes);
	if (gProject->itsOptions->useMC68881) {
	    switch (OP) {
	    case M68op_ADD:
		GenInst(M68op_FADD, GetLocSZ(rightop), rightop, leftop, Codes);
		break;
	    case M68op_SUB:
		GenInst(M68op_FSUB, GetLocSZ(rightop), rightop, leftop, Codes);
		break;
	    }
	} else {
	    LocAMVia_t                      ftemp;
	    ftemp = TempFloatReg(Codes);
	    if (isShortFloatingType(GetTreeTP(left))) {
		GenSANECall(FFSGL + FOZ2X, leftop, ftemp, Codes);
		switch (OP) {
		case M68op_ADD:
		    GenSANECall(FFSGL + FOADD, rightop, ftemp, Codes);
		    break;
		case M68op_SUB:
		    GenSANECall(FFSGL + FOSUB, rightop, ftemp, Codes);
		    break;
		}
		GenSANECall(FFSGL + FOX2Z, ftemp, leftop, Codes);
	    } else if (isExtendedFloatingType(GetTreeTP(left))) {
		GenSANECall(FFEXT + FOZ2X, leftop, ftemp, Codes);
		switch (OP) {
		case M68op_ADD:
		    GenSANECall(FFEXT + FOADD, rightop, ftemp, Codes);
		    break;
		case M68op_SUB:
		    GenSANECall(FFEXT + FOSUB, rightop, ftemp, Codes);
		    break;
		}
		GenSANECall(FFEXT + FOX2Z, ftemp, leftop, Codes);
	    } else {
		GenSANECall(FFDBL + FOZ2X, leftop, ftemp, Codes);
		switch (OP) {
		case M68op_ADD:
		    GenSANECall(FFDBL + FOADD, rightop, ftemp, Codes);
		    break;
		case M68op_SUB:
		    GenSANECall(FFDBL + FOSUB, rightop, ftemp, Codes);
		    break;
		}
		GenSANECall(FFDBL + FOX2Z, ftemp, leftop, Codes);
	    }
	}
    } else {
	/* signed/unsigned and long/short should be identical here. */
	leftop = GenExpression(left, Codes);
	rightop = GenExpression(right, Codes);
	if (GetLocAM(leftop) == M68am_ARegDisplaceFIELD) {
	    unsigned long                   bitmask;
	    unsigned long                   mask2;
	    LocAMVia_t                      prev;
	    LocAMVia_t                      val2;
	    prev = TempDataReg(Codes);
	    Genmove(M68sz_long, leftop, prev, Codes);
	    /* Construct a mask, and AND immed */
	    bitmask = MakeMask(leftop);
	    mask2 = ~bitmask;
	    GenInst(M68op_AND, GetLocSZ(leftop),
		    BuildImmediate(bitmask, M68sz_long), prev, Codes);

	    ShiftRight(prev, GetLocFieldBits(leftop).StartBit, Codes);
	    GenInst(OP, GetLocSZ(leftop), rightop, prev, Codes);
	    ShiftLeft(prev, GetLocFieldBits(leftop).StartBit, Codes);
	    val2 = TempDataReg(Codes);
	    Genmove(M68sz_long, leftop, val2, Codes);
	    /* Construct a mask, and AND immed */
	    GenInst(M68op_AND, GetLocSZ(leftop),
		    BuildImmediate(mask2, M68sz_long), val2, Codes);
	    GenInst(M68op_OR, GetLocSZ(leftop), prev, val2, Codes);
	    Genmove(M68sz_long, val2, leftop, Codes);
	    FreeTemp(prev);
	    FreeTemp(val2);
	} else {
	    rightop = DictateTempDReg(rightop, Codes);
	    GenInst(OP, GetLocSZ(leftop), rightop, leftop, Codes);
	}
    }
    FreeTemp(rightop);
    return leftop;
}

LocAMVia_t
GenPreInc(ParseTreeVia_t expr, InstListVia_t Codes)
{
    LocAMVia_t                      object;
    object = GenExpression(expr, Codes);
    if (isPointerType(GetTreeTP(expr))) {
	GenInst(M68op_ADD, M68_TypeSize(GetTreeTP(expr)),
		BuildImmediate(GetTPSize(GetTPBase(GetTreeTP(expr))), GetTPSize(GetTreeTP(expr))), object, Codes);
    } else if (isFloatingType(GetTreeTP(expr))) {
	/*
	 * Generate an integer constant of 1, do a type convert (just like in
	 * CompareZero), and add it directly to object.
	 */
	ParseTreeVia_t                  oneExpr;
	LocAMVia_t                      oneLoc;
	oneExpr = BuildTreeNode(PTF_intconstant, NULL, NULL, NULL);
	SetTreeTP(oneExpr, BuildTypeRecord(0, TRC_int, SGN_signed));
	Via(oneExpr)->data.number = 1;
	oneExpr = TypeConvert(oneExpr, BuildTypeRecord(0, TRC_longdouble,
						       SGN_unknown));
	oneLoc = GenExpression(oneExpr, Codes);
	FreeTree(oneExpr);
	if (gProject->itsOptions->useMC68881) {
	    GenInst(M68op_FADD, GetLocSZ(object), oneLoc, object, Codes);
	} else {
	    LocAMVia_t                      ftemp;
	    ftemp = TempFloatReg(Codes);
	    if (isExtendedFloatingType(GetTreeTP(expr))) {
		GenSANECall(FFEXT + FOZ2X, object, ftemp, Codes);
		GenSANECall(FFLNG + FOADD, oneLoc, ftemp, Codes);
		GenSANECall(FFEXT + FOX2Z, ftemp, object, Codes);
	    } else if (isLongFloatingType(GetTreeTP(expr))) {
		GenSANECall(FFDBL + FOZ2X, object, ftemp, Codes);
		GenSANECall(FFLNG + FOADD, oneLoc, ftemp, Codes);
		GenSANECall(FFDBL + FOX2Z, ftemp, object, Codes);
	    } else {
		GenSANECall(FFSGL + FOZ2X, object, ftemp, Codes);
		GenSANECall(FFLNG + FOADD, oneLoc, ftemp, Codes);
		GenSANECall(FFSGL + FOX2Z, ftemp, object, Codes);
	    }
	}
    } else {
	if (GetLocAM(object) == M68am_ARegDisplaceFIELD) {
	    unsigned long                   bitmask;
	    unsigned long                   mask2;
	    LocAMVia_t                      prev;
	    LocAMVia_t                      val2;
	    prev = TempDataReg(Codes);
	    Genmove(M68sz_long, object, prev, Codes);
	    /* Construct a mask, and AND immed */
	    bitmask = MakeMask(object);
	    mask2 = ~bitmask;
	    GenInst(M68op_AND, GetLocSZ(object),
		    BuildImmediate(bitmask, M68sz_long), prev, Codes);

	    ShiftRight(prev, GetLocFieldBits(object).StartBit, Codes);
	    GenInst(M68op_ADDQ, GetTPSize(GetTreeTP(expr)),
		BuildImmediate(1, GetTPSize(GetTreeTP(expr))), prev, Codes);
	    ShiftLeft(prev, GetLocFieldBits(object).StartBit, Codes);
	    val2 = TempDataReg(Codes);
	    Genmove(M68sz_long, object, val2, Codes);
	    GenInst(M68op_AND, GetLocSZ(object),
		    BuildImmediate(mask2, M68sz_long), val2, Codes);
	    GenInst(M68op_OR, GetLocSZ(object), prev, val2, Codes);
	    Genmove(M68sz_long, val2, object, Codes);
	    FreeTemp(prev);
	    FreeTemp(val2);
	} else {
	    GenInst(M68op_ADDQ, GetTPSize(GetTreeTP(expr)),
	      BuildImmediate(1, GetTPSize(GetTreeTP(expr))), object, Codes);
	}
    }
    return object;
}

LocAMVia_t
GenPreDec(ParseTreeVia_t expr, InstListVia_t Codes)
{
    LocAMVia_t                      object;
    object = GenExpression(expr, Codes);
    if (isPointerType(GetTreeTP(expr))) {
	GenInst(M68op_SUB, GetTPSize(GetTreeTP(expr)),
		BuildImmediate(GetTPSize(GetTPBase(GetTreeTP(expr))),
			       GetTPSize(GetTreeTP(expr))), object, Codes);
    } else if (isFloatingType(GetTreeTP(expr))) {
	/*
	 * Generate an integer constant of 1, do a type convert (just like in
	 * CompareZero), and sub it directly to object.
	 */
	ParseTreeVia_t                  oneExpr;
	LocAMVia_t                      oneLoc;
	oneExpr = BuildTreeNode(PTF_intconstant, NULL, NULL, NULL);
	SetTreeTP(oneExpr, BuildTypeRecord(0, TRC_int, SGN_signed));
	Via(oneExpr)->data.number = 1;
	oneExpr = TypeConvert(oneExpr, BuildTypeRecord(0, TRC_longdouble,
						       SGN_unknown));
	oneLoc = GenExpression(oneExpr, Codes);
	FreeTree(oneExpr);
	if (gProject->itsOptions->useMC68881) {
	    GenInst(M68op_FSUB, GetLocSZ(object), oneLoc, object, Codes);
	} else {
	    LocAMVia_t                      ftemp;
	    ftemp = TempFloatReg(Codes);
	    if (isExtendedFloatingType(GetTreeTP(expr))) {
		GenSANECall(FFEXT + FOZ2X, object, ftemp, Codes);
		GenSANECall(FFLNG + FOSUB, oneLoc, ftemp, Codes);
		GenSANECall(FFEXT + FOX2Z, ftemp, object, Codes);
	    } else if (isLongFloatingType(GetTreeTP(expr))) {
		GenSANECall(FFDBL + FOZ2X, object, ftemp, Codes);
		GenSANECall(FFLNG + FOSUB, oneLoc, ftemp, Codes);
		GenSANECall(FFDBL + FOX2Z, ftemp, object, Codes);
	    } else {
		GenSANECall(FFSGL + FOZ2X, object, ftemp, Codes);
		GenSANECall(FFLNG + FOSUB, oneLoc, ftemp, Codes);
		GenSANECall(FFSGL + FOX2Z, ftemp, object, Codes);
	    }
	}
    } else {
	if (GetLocAM(object) == M68am_ARegDisplaceFIELD) {
	    unsigned long                   bitmask;
	    unsigned long                   mask2;
	    LocAMVia_t                      prev;
	    LocAMVia_t                      val2;
	    prev = TempDataReg(Codes);
	    Genmove(M68sz_long, object, prev, Codes);
	    /* Construct a mask, and AND immed */
	    bitmask = MakeMask(object);
	    mask2 = ~bitmask;
	    GenInst(M68op_AND, GetLocSZ(object),
		    BuildImmediate(bitmask, M68sz_long), prev, Codes);

	    ShiftRight(prev, GetLocFieldBits(object).StartBit, Codes);
	    GenInst(M68op_SUBQ, GetTPSize(GetTreeTP(expr)),
		BuildImmediate(1, GetTPSize(GetTreeTP(expr))), prev, Codes);
	    ShiftLeft(prev, GetLocFieldBits(object).StartBit, Codes);
	    val2 = TempDataReg(Codes);
	    Genmove(M68sz_long, object, val2, Codes);
	    GenInst(M68op_AND, GetLocSZ(object),
		    BuildImmediate(mask2, M68sz_long), val2, Codes);
	    GenInst(M68op_OR, GetLocSZ(object), prev, val2, Codes);
	    Genmove(M68sz_long, val2, object, Codes);
	    FreeTemp(prev);
	    FreeTemp(val2);
	} else {
	    GenInst(M68op_SUBQ, GetTPSize(GetTreeTP(expr)),
	      BuildImmediate(1, GetTPSize(GetTreeTP(expr))), object, Codes);
	}
    }
    return object;
}

LocAMVia_t
GenPostInc(ParseTreeVia_t expr, InstListVia_t Codes)
{
    LocAMVia_t                      result;
    LocAMVia_t                      object;
    object = GenExpression(expr, Codes);
    if (isPointerType(GetTreeTP(expr))) {
	/*
	 * What we really care about with this call to Dictate... is that
	 * result is NOT the same mode as object.  We want to modify the
	 * object, but not the result.
	 */
	result = TempAddrReg(Codes);
	Genmove(M68sz_long, object, result, Codes);
	GenInst(M68op_ADD, GetTPSize(GetTreeTP(expr)),
		BuildImmediate(GetTPSize(GetTPBase(GetTreeTP(expr))),
			       GetTPSize(GetTreeTP(expr))), object, Codes);
	FreeTemp(object);
    } else if (isFloatingType(GetTreeTP(expr))) {
	/*
	 * Generate an integer constant of 1, do a type convert (just like in
	 * CompareZero), and add it to object after moving its pre value to a
	 * float reg.
	 */
	ParseTreeVia_t                  oneExpr;
	LocAMVia_t                      oneLoc;
	result = TempFloatReg(Codes);
	Genmove(GetLocSZ(object), object, result, Codes);
	oneExpr = BuildTreeNode(PTF_intconstant, NULL, NULL, NULL);
	SetTreeTP(oneExpr, BuildTypeRecord(0, TRC_int, SGN_signed));
	Via(oneExpr)->data.number = 1;
	oneExpr = TypeConvert(oneExpr, BuildTypeRecord(0, TRC_longdouble,
						       SGN_unknown));
	oneLoc = GenExpression(oneExpr, Codes);
	FreeTree(oneExpr);
	if (gProject->itsOptions->useMC68881) {
	    GenInst(M68op_FADD, GetLocSZ(object), oneLoc, object, Codes);
	} else {
	    LocAMVia_t                      ftemp;
	    ftemp = TempFloatReg(Codes);
	    if (isExtendedFloatingType(GetTreeTP(expr))) {
		GenSANECall(FFEXT + FOZ2X, object, ftemp, Codes);
		GenSANECall(FFLNG + FOADD, oneLoc, ftemp, Codes);
		GenSANECall(FFEXT + FOX2Z, ftemp, object, Codes);
	    } else if (isLongFloatingType(GetTreeTP(expr))) {
		GenSANECall(FFDBL + FOZ2X, object, ftemp, Codes);
		GenSANECall(FFLNG + FOADD, oneLoc, ftemp, Codes);
		GenSANECall(FFDBL + FOX2Z, ftemp, object, Codes);
	    } else {
		GenSANECall(FFSGL + FOZ2X, object, ftemp, Codes);
		GenSANECall(FFLNG + FOADD, oneLoc, ftemp, Codes);
		GenSANECall(FFSGL + FOX2Z, ftemp, object, Codes);
	    }
	}
    } else {
	result = TempDataReg(Codes);
	if (GetLocAM(object) == M68am_ARegDisplaceFIELD) {
	    unsigned long                   bitmask;
	    unsigned long                   mask2;
	    LocAMVia_t                      prev;
	    LocAMVia_t                      val2;
	    prev = TempDataReg(Codes);
	    Genmove(M68sz_long, object, prev, Codes);
	    /* Construct a mask, and AND immed */
	    bitmask = MakeMask(object);
	    mask2 = ~bitmask;
	    GenInst(M68op_AND, GetLocSZ(object),
		    BuildImmediate(bitmask, M68sz_long), prev, Codes);

	    ShiftRight(prev, GetLocFieldBits(object).StartBit, Codes);
	    Genmove(GetLocSZ(object), prev, result, Codes);
	    GenInst(M68op_ADDQ, GetTPSize(GetTreeTP(expr)),
		BuildImmediate(1, GetTPSize(GetTreeTP(expr))), prev, Codes);
	    ShiftLeft(prev, GetLocFieldBits(object).StartBit, Codes);
	    val2 = TempDataReg(Codes);
	    Genmove(M68sz_long, object, val2, Codes);
	    GenInst(M68op_AND, GetLocSZ(object),
		    BuildImmediate(mask2, M68sz_long), val2, Codes);
	    GenInst(M68op_OR, GetLocSZ(object), prev, val2, Codes);
	    Genmove(M68sz_long, val2, object, Codes);
	    FreeTemp(prev);
	    FreeTemp(val2);
	} else {
	    Genmove(GetLocSZ(object), object, result, Codes);
	    GenInst(M68op_ADDQ, GetTPSize(GetTreeTP(expr)),
	      BuildImmediate(1, GetTPSize(GetTreeTP(expr))), object, Codes);
	}
	FreeTemp(object);
    }
    SetLocSZ(result, M68_TypeSize(GetTreeTP(expr)));
    return result;
}

LocAMVia_t
GenPostDec(ParseTreeVia_t expr, InstListVia_t Codes)
{
    LocAMVia_t                      result;
    LocAMVia_t                      object;
    object = GenExpression(expr, Codes);
    if (isPointerType(GetTreeTP(expr))) {
	result = TempAddrReg(Codes);
	Genmove(GetLocSZ(object), object, result, Codes);
	GenInst(M68op_SUB, GetTPSize(GetTreeTP(expr)),
		BuildImmediate(GetTPSize(GetTPBase(GetTreeTP(expr))),
			       GetTPSize(GetTreeTP(expr))), object, Codes);
	FreeTemp(object);
    } else if (isFloatingType(GetTreeTP(expr))) {
	/*
	 * Generate an integer constant of 1, do a type convert (just like in
	 * CompareZero), and sub it to object after moving its pre value to a
	 * float reg.
	 */
	ParseTreeVia_t                  oneExpr;
	LocAMVia_t                      oneLoc;
	result = TempFloatReg(Codes);
	Genmove(GetLocSZ(object), object, result, Codes);
	oneExpr = BuildTreeNode(PTF_intconstant, NULL, NULL, NULL);
	SetTreeTP(oneExpr, BuildTypeRecord(0, TRC_int, SGN_signed));
	Via(oneExpr)->data.number = 1;
	oneExpr = TypeConvert(oneExpr, BuildTypeRecord(0, TRC_longdouble,
						       SGN_unknown));
	oneLoc = GenExpression(oneExpr, Codes);
	FreeTree(oneExpr);
	if (gProject->itsOptions->useMC68881) {
	    GenInst(M68op_FSUB, GetLocSZ(object), oneLoc, object, Codes);
	} else {
	    LocAMVia_t                      ftemp;
	    ftemp = TempFloatReg(Codes);
	    if (isExtendedFloatingType(GetTreeTP(expr))) {
		assert(0);
		GenSANECall(FFDBL + FOZ2X, object, ftemp, Codes);
		GenSANECall(FFLNG + FOSUB, oneLoc, ftemp, Codes);
		GenSANECall(FFEXT + FOX2Z, ftemp, object, Codes);
	    } else if (isLongFloatingType(GetTreeTP(expr))) {
		GenSANECall(FFDBL + FOZ2X, object, ftemp, Codes);
		GenSANECall(FFLNG + FOSUB, oneLoc, ftemp, Codes);
		GenSANECall(FFDBL + FOX2Z, ftemp, object, Codes);
	    } else {
		GenSANECall(FFSGL + FOZ2X, object, ftemp, Codes);
		GenSANECall(FFLNG + FOSUB, oneLoc, ftemp, Codes);
		GenSANECall(FFSGL + FOX2Z, ftemp, object, Codes);
	    }
	}
    } else {
	result = TempDataReg(Codes);
	if (GetLocAM(object) == M68am_ARegDisplaceFIELD) {
	    unsigned long                   bitmask;
	    unsigned long                   mask2;
	    LocAMVia_t                      prev;
	    LocAMVia_t                      val2;
	    prev = TempDataReg(Codes);
	    Genmove(M68sz_long, object, prev, Codes);
	    /* Construct a mask, and AND immed */
	    bitmask = MakeMask(object);
	    mask2 = ~bitmask;
	    GenInst(M68op_AND, GetLocSZ(object),
		    BuildImmediate(bitmask, M68sz_long), prev, Codes);

	    ShiftRight(prev, GetLocFieldBits(object).StartBit, Codes);
	    Genmove(GetLocSZ(object), prev, result, Codes);
	    GenInst(M68op_SUBQ, GetTPSize(GetTreeTP(expr)),
		BuildImmediate(1, GetTPSize(GetTreeTP(expr))), prev, Codes);
	    ShiftLeft(prev, GetLocFieldBits(object).StartBit, Codes);
	    val2 = TempDataReg(Codes);
	    Genmove(M68sz_long, object, val2, Codes);
	    GenInst(M68op_AND, GetLocSZ(object),
		    BuildImmediate(mask2, M68sz_long), val2, Codes);
	    GenInst(M68op_OR, GetLocSZ(object), prev, val2, Codes);
	    Genmove(M68sz_long, val2, object, Codes);
	    FreeTemp(prev);
	    FreeTemp(val2);
	} else {
	    Genmove(GetLocSZ(object), object, result, Codes);
	    GenInst(M68op_SUBQ, GetTPSize(GetTreeTP(expr)),
	      BuildImmediate(1, GetTPSize(GetTreeTP(expr))), object, Codes);
	}
	FreeTemp(object);
    }
    SetLocSZ(result, M68_TypeSize(GetTreeTP(expr)));
    return result;
}
