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
 * This file contains routines for 68k code generation. These routines generate
 * code for expressions.
 * 
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

#pragma segment GenExpr


void
GenTrapWordsRecurse(ParseTreeVia_t init, InstListVia_t Codes)
{
    FoldValue_t                     thek;
	if (init) {
		if (Via(init)->b) {
			GenTrapWordsRecurse(Via(init)->b,Codes);
		}
		ConstExprValue(init, &thek);
		GenInst(M68op_DC, M68sz_word,BuildAbsolute(thek.intval), NULL, Codes);
	}
}

void
GenTrapWords(ParseTreeVia_t init, InstListVia_t Codes)
{
    FoldValue_t                     thek;
    if (Via(init)->kind == PTF_multi_initializer) {
		init = Via(init)->a;
		GenTrapWordsRecurse(init,Codes);	/* They're backwards */
    } else {
		ConstExprValue(init, &thek);
		GenInst(M68op_DC, M68sz_word,BuildAbsolute(thek.intval), NULL, Codes);
    }
}

LocAMVia_t
ParamLoc(int x)
{
    switch (x) {
    case param__A0:
	return BuildARegDirect(0);
	break;
    case param__A1:
	return BuildARegDirect(1);
	break;
    case param__D0:
	return BuildDRegLocation(0);
	break;
    case param__D1:
	return BuildDRegLocation(0);
	break;
    case param__D2:
	return BuildDRegLocation(0);
	break;
    case 0:
    default:
	return NULL;
    }
}

LocAMVia_t
GenFunctionCall(ParseTreeVia_t expr, InstListVia_t Codes)
{
    LocAMVia_t                      result;
    LocAMVia_t                      ParamReturn;
    LabSYMVia_t                     funcname;
    TypeRecordVia_t                 functype;
    LocAMVia_t                      funcloc;
    int                             SumArgs;
    SpillSlotVia_t                  bigresult;
    SpillSlotVia_t                  ptrslot;
    int                             discardit;
    discardit = 0;
    result = NULL;
    ParamReturn = NULL;
    /*
     * Discarded function call indicates that the result of this function is
     * not needed, so we will not bother to allocate a place to store it.
     */
    if (DiscardedFunctionCall) {
		DiscardedFunctionCall = 0;
		discardit = 1;
    }
    /*
     * Struct valued functions are tricky.  We cannot return a struct in D0,
     * so we pass the called function a pointer to a place to put it.
     */
    bigresult = NULL;
    if (isStructUnionType(GetTreeTP(expr)) && (GetTPSize(GetTreeTP(expr)) > 4)) {
		bigresult = GetSpillSlot(GetTPSize(GetTreeTP(expr)));
    }
    functype = Via(expr)->data.TP;
    if (!functype) {
		Gen68Error("Generating function call for non function");
    }
    SaveTheRegisters(Codes);
    if (GetTPParam(functype)) {
		if (Via(GetTPParam(functype))->returnreg)
		    ParamReturn = ParamLoc(Via(GetTPParam(functype))->returnreg);
    }
    if (GetTPFlags(functype) & ISPASCALMASK) {
	/*
	 * If this is a pascal function, we must allocate space on the stack
	 * for the result
	 */
	if (!ParamReturn)
	    if (!(isVoidType(GetTreeTP(expr)))) {
		if (GetTPSize(GetTreeTP(expr))) {	/* TODO Disallow returns
							 * of struct/double for
							 * pascal */
		    if (GetTPSize(GetTreeTP(expr)) == 1)
			GenInst(M68op_SUB, M68sz_long,
				BuildImmediate(2, M68sz_long),
				BuildARegDirect(7), Codes);
		    else
			GenInst(M68op_SUB, M68sz_long,
				BuildImmediate(GetTPSize(GetTreeTP(expr)), M68sz_long),
				BuildARegDirect(7), Codes);
		}
	    }
    }
    /*
     * Push arguments onto the stack.  Each argument is evaluated before
     * being pushed, in PushArguments()
     */
    SumArgs = PushArguments(expr, (GetTPFlags(functype) &
				ISPASCALMASK), GetTPParam(functype), Codes);
    /* Here, all registers should be free. */
    /*
     * Generate the jsr instruction. The handling of relative branches is not
     * currently done here. For the Mac, this jsr will have to be converted
     * to a jsr (jumptable)a5 address.
     */
    /*
     * If the routine is struct/double valued function, we allocate a spill
     * zone to contain the result, and push a pointer to that spill zone as
     * the LAST argument pushed.  The Sun handles this by moving the pointer
     * into A1 instead.
     */
    if (isStructUnionType(GetTreeTP(expr)) && (GetTPSize(GetTreeTP(expr)) > 4)) {
		GenPushEA(Via(bigresult)->SlotLoc, Codes);
		SumArgs += 4;
    }
    funcname = NULL;
    funcloc = NULL;
    /* This code is UGLY */
    if (Via(Via(expr)->a)->kind == PTF_gen_addrof) {
		if (Via(Via(Via(expr)->a)->a)->kind == PTF_identifier) {
		    funcloc = Via(Via(Via(Via(expr)->a)->a)->data.thesymbol)->M68kDef.Loc;
		}
    }
    if (!funcloc) {
		LocAMVia_t                      thru;
		LocAMVia_t                      ind;
		thru = GenExpression(Via(expr)->a, Codes);
		ind = TempAddrReg(Codes);
		Genmove(GetLocSZ(thru), thru, ind, Codes);
		funcloc = BuildARegIndirect(GetLocAReg(ind));
    }
    if (!(GetTPTrap(functype))) {
		GenInst(M68op_JSR, M68sz_none,funcloc, NULL, Codes);
    } else {
		GenTrapWords(GetTPTrap(functype), Codes);
    }
    FreeTemp(funcloc);
    /* Fix the stack... */
    if (SumArgs) {
		if (!(GetTPFlags(functype) & ISPASCALMASK)) {
		    /* MOTOM ADD Should be ADDA */
		    GenInst(M68op_ADD, M68sz_long,
			    BuildImmediate(SumArgs, M68sz_long),
			    BuildARegDirect(7), Codes);
		}
    }
    /* Now do something about the return value */
    if (isStructUnionType(GetTreeTP(expr)) && (GetTPSize(GetTreeTP(expr)) > 4)) {
		result = Via(bigresult)->SlotLoc;
    } else if (isFloatingType(GetTreeTP(expr))) {
		/*
		 * Allocate a spill slot, and move the contents of d0-d1-a0 to it.
		 * This follows the return conventions of the MPW C compiler,
		 * documented in the MPW C manual appendix C, page 187 version 3.1
		 */
		LocAMVia_t                      half2;
		bigresult = GetSpillSlot(SizeOfLongDouble);
		Genmove(M68sz_word, BuildDRegLocation(0), Via(bigresult)->SlotLoc, Codes);
		half2 = BuildARegDisplace(GetLocAReg(Via(bigresult)->SlotLoc),
				       GetLocConstant(Via(bigresult)->SlotLoc) + 2);
		Genmove(M68sz_long, BuildDRegLocation(1), half2,
			Codes);
		half2 = BuildARegDisplace(GetLocAReg(Via(bigresult)->SlotLoc),
				       GetLocConstant(Via(bigresult)->SlotLoc) + 6);
		Genmove(M68sz_long, BuildARegDirect(0), half2,
			Codes);
		result = Via(bigresult)->SlotLoc;
		SetLocSZ(result, M68_TypeSize(GetTreeTP(expr)));
    } else if (discardit || isVoidType(GetTreeTP(expr))) {
		result = NULL;
    } else {
		/*
		 * Function results are always returned in D0. Pascal calling
		 * conventions are an exception.
		 */
		if (ParamReturn) {
		    result = TempDataReg(Codes);
		    SetLocSZ(result, M68_TypeSize(GetTreeTP(expr)));
		    Genmove(GetLocSZ(result), ParamReturn, result, Codes);
		} else {
		    if ((GetTPFlags(functype) & ISPASCALMASK)) {
				LocAMVia_t                      spinc;
				spinc = BuildARegPostInc(7);
				result = TempDataReg(Codes);
				SetLocSZ(result, M68_TypeSize(GetTreeTP(expr)));
				Genmove(GetLocSZ(result), spinc, result, Codes);
				if (GetLocSZ(result) == M68sz_byte) {
				    GenInst(M68op_EXT, M68sz_word, result, NULL, Codes);
				    GenInst(M68op_EXT, M68sz_long, result, NULL, Codes);
					SetLocSZ(result, M68sz_long);
				}
				if (GetLocSZ(result) == M68sz_word) {
				    GenInst(M68op_EXT, M68sz_long, result, NULL, Codes);
					SetLocSZ(result, M68sz_long);
				}
		    } else {
				result = TempDataReg(Codes);
				if (GetLocDReg(result) != 0) {
				    Genmove(M68_TypeSize(GetTreeTP(expr)), BuildDRegLocation(0), result, Codes);
				}
				SetLocSZ(result, M68_TypeSize(GetTreeTP(expr)));
				if (GetLocSZ(result) == M68sz_byte) {
				    GenInst(M68op_EXT, M68sz_word, result, NULL, Codes);
				    GenInst(M68op_EXT, M68sz_long, result, NULL, Codes);
					SetLocSZ(result, M68sz_long);
				}
				if (GetLocSZ(result) == M68sz_word) {
				    GenInst(M68op_EXT, M68sz_long, result, NULL, Codes);
					SetLocSZ(result, M68sz_long);
				}
		    }
		}
    }
    return result;
}

LocAMVia_t
GenLogicalOr(ParseTreeVia_t left, ParseTreeVia_t right,
	     InstListVia_t Codes)
{
    LocAMVia_t                      result;
    LabSYMVia_t                     Maketrue;
    LabSYMVia_t                     DoneLabel;
    Maketrue = MakeEccLabel(NextEccLabel++);
    DoneLabel = MakeEccLabel(NextEccLabel++);

    GenCompareNonZero(left, Maketrue, Codes);
    GenCompareNonZero(right, Maketrue, Codes);
    result = TempDataReg(Codes);
    SetLocSZ(result, M68sz_long);
    GenInst(M68op_MOVEQ, M68sz_none, BuildImmediate(0, M68sz_long), result, Codes);
    GenInst(M68op_BRA, M68sz_none, BuildLabelLoc(DoneLabel), NULL, Codes);
    GenLabel(Maketrue, Codes);
    GenInst(M68op_MOVEQ, M68sz_none,
	    BuildImmediate(1, M68sz_long),
	    result, Codes);
    GenLabel(DoneLabel, Codes);
    if (gProject->itsOptions->int2byte)
	SetLocSZ(result, 2);
    else
	SetLocSZ(result, 4);
    return result;
}

LocAMVia_t
GenLogicalAnd(ParseTreeVia_t left, ParseTreeVia_t right,
	      InstListVia_t Codes)
{
    LocAMVia_t                      result;
    LabSYMVia_t                     Makefalse;
    LabSYMVia_t                     DoneLabel;
    Makefalse = MakeEccLabel(NextEccLabel++);
    DoneLabel = MakeEccLabel(NextEccLabel++);

    GenCompareZero(left, Makefalse, Codes);
    GenCompareZero(right, Makefalse, Codes);
    result = TempDataReg(Codes);
    SetLocSZ(result, M68sz_long);
    GenInst(M68op_MOVEQ, M68sz_none,
	    BuildImmediate(1, M68sz_long),
	    result, Codes);
    GenInst(M68op_BRA, M68sz_none, BuildLabelLoc(DoneLabel), NULL, Codes);
    GenLabel(Makefalse, Codes);
    GenInst(M68op_MOVEQ, M68sz_none, BuildImmediate(0, M68sz_long), result, Codes);
    GenLabel(DoneLabel, Codes);
    if (gProject->itsOptions->int2byte)
	SetLocSZ(result, 2);
    else
	SetLocSZ(result, 4);
    return result;
}

LocAMVia_t
GenTernary(ParseTreeVia_t cond,
	   ParseTreeVia_t trueval,
	   ParseTreeVia_t falseval,
	   InstListVia_t Codes)
{
    LocAMVia_t                      result;
    LocAMVia_t                      exprop;
    LabSYMVia_t                     Makefalse;
    LabSYMVia_t                     DoneLabel;
    SpillSlotVia_t                  slot;
    /*
     * The temp value here is held in a spill slot instead of a register to
     * avoid problems with reg allocation.
     */
    slot = GetSpillSlot(M68_TypeSize(GetTreeTP(trueval)));
    result = Via(slot)->SlotLoc;
    SetLocSZ(result, M68sz_long);
    Makefalse = MakeEccLabel(NextEccLabel++);
    DoneLabel = MakeEccLabel(NextEccLabel++);
    GenCompareZero(cond, Makefalse, Codes);
    exprop = GenExpression(trueval, Codes);
    Genmove(M68_TypeSize(GetTreeTP(trueval)), exprop, result, Codes);
    FreeTemp(exprop);
    GenInst(M68op_BRA, M68sz_none, BuildLabelLoc(DoneLabel), NULL, Codes);
    GenLabel(Makefalse, Codes);
    exprop = GenExpression(falseval, Codes);
    Genmove(M68_TypeSize(GetTreeTP(falseval)), exprop, result, Codes);
    FreeTemp(exprop);
    GenLabel(DoneLabel, Codes);
    SetLocSZ(result, M68_TypeSize(GetTreeTP(trueval)));
    return result;
}

LocAMVia_t
GenLogicalNegate(ParseTreeVia_t expr, InstListVia_t Codes)
{
    LocAMVia_t                      result;
    LabSYMVia_t                     Makefalse;
    LabSYMVia_t                     DoneLabel;
    Makefalse = MakeEccLabel(NextEccLabel++);
    DoneLabel = MakeEccLabel(NextEccLabel++);
    GenCompareNonZero(expr, Makefalse, Codes);
    result = TempDataReg(Codes);
    SetLocSZ(result, M68sz_long);
    GenInst(M68op_MOVEQ, M68sz_none,
	    BuildImmediate(1, M68sz_long), result, Codes);
    GenInst(M68op_BRA, M68sz_none,
	    BuildLabelLoc(DoneLabel), NULL, Codes);
    GenLabel(Makefalse, Codes);
    GenInst(M68op_MOVEQ, M68sz_none, BuildImmediate(0, M68sz_long), result, Codes);
    GenLabel(DoneLabel, Codes);
    SetLocSZ(result, M68_TypeSize(GetTreeTP(expr)));
    return result;
}

LocAMVia_t
GenRelational(Opcode_t OP, ParseTreeVia_t left, ParseTreeVia_t right,
	      InstListVia_t Codes)
{
    LabSYMVia_t                     TrueLabelSym;
    LabSYMVia_t                     EndLabelSym;
    LocAMVia_t                      result;
    /*
     * The OP argument is the Bcc instruction which is appropriate for
     * whatever relational we are doing.
     */
    /* The only possibilities for OP are BEQ,BNE,BLE,BGE,BLT,BGT */


    TrueLabelSym = MakeEccLabel(NextEccLabel++);
    EndLabelSym = MakeEccLabel(NextEccLabel++);

    GenCompareBranch(left, right, OP, TrueLabelSym, Codes);

    /* QQQQ Can we used Scc instead of this mess ? */
    /* This is the false case - we clear the result register. */
    result = TempDataReg(Codes);

    GenInst(M68op_MOVEQ, M68sz_none, BuildImmediate(0, M68sz_long), result, Codes);
    /* Jump over the true case. */
    GenInst(M68op_BRA, M68sz_none,
	    BuildLabelLoc(EndLabelSym), NULL, Codes);
    GenLabel(TrueLabelSym, Codes);
    /* This is the true case. */
    GenInst(M68op_MOVEQ, M68sz_none,
	    BuildImmediate(1, M68sz_long), result, Codes);
    GenLabel(EndLabelSym, Codes);
    if (gProject->itsOptions->int2byte)
	SetLocSZ(result, 2);
    else
	SetLocSZ(result, 4);
    return result;
}

LocAMVia_t
GenbitTwoOp(Opcode_t OP,
	    ParseTreeVia_t left, ParseTreeVia_t right, InstListVia_t Codes)
{
    /* Integers only */
    LocAMVia_t                      leftop;
    LocAMVia_t                      rightop;
    leftop = GenExpression(left, Codes);
    rightop = GenExpression(right, Codes);
    leftop = DictateTempDReg(leftop, Codes);
    rightop = DictateAnyDReg(rightop, Codes);
    /*
     * Careful with the Size specification here !  This could be all wrong
     * with respect to shifts and legal addressingmodes - same for >>= and
     * <<= elsewhere in the code...
     */
    GenDeConflictize(&leftop, &rightop, Codes);
    GenInst(OP, GetLocSZ(leftop), rightop, leftop, Codes);
    FreeTemp(rightop);
    return leftop;
}

/*
 * General notes about SANE math.  For any SANE call, we must push both
 * operands, with the destination on top (ie push src first, dest second).
 * These operands must be in the form of a pointer to a range of memory
 * holding the floating point number.  In the case of the destination, this
 * must always be an extended number. Therefore, destination operands for
 * SANE calls must always be a float temp spill slot.  The format of the
 * source operand may be extended, double, single, 16bit int, or 32 bit int.
 */
LocAMVia_t
GenTwoOp(Opcode_t OP,
	 ParseTreeVia_t left, ParseTreeVia_t right, InstListVia_t Codes)
{
    /* The possibilities for OP are ADD, SUB, AND, OR */
    LocAMVia_t                      result;
    LocAMVia_t                      leftop;
    LocAMVia_t                      rightop;
    LocAMVia_t                      multiplier;
    rightop = GenExpression(right, Codes);
    leftop = GenExpression(left, Codes);

    if (isFloatingType(GetTreeTP(left))) {
	/*
	 * In the case of floating math, we need to switch on OP to find out
	 * what ops we should really generate.
	 */
	leftop = DictateTempFloat(leftop, Codes);	/* Make sure leftop is
							 * in a temp. */
	if (gProject->itsOptions->useMC68881) {
	    switch (OP) {
	    case M68op_ADD:
		GenInst(M68op_FADD, GetLocSZ(rightop), rightop, leftop,
			Codes);
		break;
	    case M68op_SUB:
		GenInst(M68op_FSUB, GetLocSZ(rightop), rightop, leftop,
			Codes);
		break;
	    }
	} else {
	    rightop = DictateTempFloat(rightop, Codes);
	    switch (OP) {
	    case M68op_ADD:
		GenSANECall(FFEXT + FOADD, rightop, leftop, Codes);
		break;
	    case M68op_SUB:
		GenSANECall(FFEXT + FOSUB, rightop, leftop, Codes);
		break;
	    }
	}
	FreeTemp(rightop);
	result = leftop;
    } else {
	if (isUnsignedType(GetTreeTP(left))) {
	    leftop = DictateTempDReg(leftop, Codes);
	    GenDeConflictize(&leftop, &rightop, Codes);
	    GenInst(OP, GetLocSZ(leftop), rightop, leftop, Codes);
	    FreeTemp(rightop);
	    result = leftop;
	} else if (isPointerType(GetTreeTP(left))) {
	    if (isIntegralType(GetTreeTP(right))) {
		leftop = DictateTempAReg(leftop, Codes);
		rightop = DictateTempDReg(rightop, Codes);
		if (GetTPSize(GetTPBase(GetTreeTP(left))) != 1) {
		    multiplier = BuildImmediate(GetTPSize(GetTPBase(GetTreeTP(left))),
						M68sz_long);
		    multiplier = DictateTempDReg(multiplier, Codes);
		    rightop = GenMulSigned(M68op_MULS, rightop, multiplier, Codes);
		    FreeTemp(multiplier);
		}
		GenDeConflictize(&leftop, &rightop, Codes);
		GenInst(OP, M68sz_long, rightop, leftop, Codes);
		FreeTemp(rightop);
		result = leftop;
	    } else {
		leftop = DictateTempAReg(leftop, Codes);
		GenDeConflictize(&leftop, &rightop, Codes);
		GenInst(OP, M68sz_long, rightop, leftop, Codes);
		FreeTemp(rightop);
		result = leftop;
	    }
	} else if (isPointerType(GetTreeTP(right))) {
	    if (isIntegralType(GetTreeTP(left))) {
		rightop = DictateTempAReg(rightop, Codes);
		leftop = DictateTempDReg(leftop, Codes);
		if (GetTPSize(GetTPBase(GetTreeTP(right))) != 1) {
		    multiplier = BuildImmediate(GetTPSize(GetTPBase(GetTreeTP(right))),
						M68sz_long);
		    multiplier = DictateTempDReg(multiplier, Codes);
		    leftop = GenMulSigned(M68op_MULS, leftop, multiplier, Codes);
		    FreeTemp(multiplier);
		}
		GenDeConflictize(&leftop, &rightop, Codes);
		GenInst(OP, M68sz_long, leftop, rightop, Codes);
		FreeTemp(rightop);
		result = leftop;
	    } else {
		rightop = DictateTempAReg(rightop, Codes);
		GenDeConflictize(&leftop, &rightop, Codes);
		GenInst(OP, M68sz_long, leftop, rightop, Codes);
		FreeTemp(leftop);
		result = rightop;
	    }
	} else {
	    leftop = DictateTempDReg(leftop, Codes);
	    GenDeConflictize(&leftop, &rightop, Codes);
	    GenInst(OP, GetLocSZ(leftop), rightop, leftop, Codes);
	    FreeTemp(rightop);
	    result = leftop;
	}
    }
    return result;
}

/*
 * The subject of 32 bit multiplication and division.  ECC generates, by
 * default, code which will work on a 68010 processor. The 68010 does not
 * support 32 bit mult/div.  Therefore, something special must be done to
 * handle these cases.  The routine GenLibraryCall, appearing below, is the
 * solution when running under Sun UNIX.  It accepts the two operands as
 * arguments, and a label corresponding to a subroutine defined elsewhere. It
 * moves the left operand into D0 and the right operand into D1, and calls
 * the routine, anticipating a result in D0, which it returns in a spill
 * slot.  The other possibilities for handling long math are : use 16 bit
 * math (generally undesirable), use the 68020 muls.l instruction (this
 * should be a compiler option), use some other library orutine other than
 * Sun's. This third possibility will have to be the default when this
 * compiler becomes a real Mac compiler.  I will probably have to code these
 * routines myself.  I have two examples from which I can refer: the c68
 * libraries from the MINIX compiler, and the libraries for PDC.  In any
 * case, there are 8 occasions in the code generator wherein long mult/div
 * must be done.  Actually there are four occasions, each of them needing to
 * be handled both for signed and unsigned math : modassign (%=), modulo (%),
 * muldiv (* or /), and muldivassign (*= or /=).  All four methods for
 * generating long mul/div should be available at each of these 8 sites. In
 * addition, a decision needs to be made about the default conditions, and
 * which options take precedence over which others.  For example, when the
 * compiler is in Macintosh mode (options for this mode need to be created
 * yet), the default will be to generate calls to the libraries appropriate
 * for the Mac (which I may have to write).  However, if the use68020 switch
 * is on, we should generate muls.l instructions, even though we are in Mac
 * mode.
 */



LocAMVia_t
GenModulo(ParseTreeVia_t left,
	  ParseTreeVia_t right, InstListVia_t Codes)
{
    /* Integer only. */
    LocAMVia_t                      leftop;
    LocAMVia_t                      rightop;
    leftop = GenExpression(left, Codes);
    rightop = GenExpression(right, Codes);
    if (isUnsignedType(GetTreeTP(left))) {
	if (GetTPSize(GetTreeTP(left)) > 2) {
	    if (!gProject->itsOptions->useMC68020) {
		leftop = GenLibraryCall(leftop, rightop,
					LibLabelULMODT, Codes);
		FreeTemp(rightop);
	    } else {
		/*
		 * We currently do long mod with library even in 68020 mode.
		 */
		leftop = GenLibraryCall(leftop, rightop,
					LibLabelULMODT, Codes);
		FreeTemp(rightop);
	    }
	} else {
	    leftop = DictateTempDReg(leftop, Codes);
	    rightop = DictateTempDReg(rightop, Codes);
	    GenInst(M68op_DIVU, M68sz_none, rightop, leftop, Codes);
	    GenInst(M68op_SWAP, M68sz_none, leftop, NULL, Codes);
	    GenInst(M68op_EXT, M68sz_long, leftop, NULL, Codes);
	    FreeTemp(rightop);
	}
    } else {
	if (GetTPSize(GetTreeTP(left)) > 2) {
	    if (!gProject->itsOptions->useMC68020) {
		leftop = GenLibraryCall(leftop, rightop,
					LibLabelLMODT, Codes);
	    } else {
		/*
		 * We currently do long mod with library even in 68020 mode.
		 */
		leftop = GenLibraryCall(leftop, rightop,
					LibLabelLMODT, Codes);
	    }
	} else {
	    leftop = DictateTempDReg(leftop, Codes);
	    rightop = DictateTempDReg(rightop, Codes);
	    GenInst(M68op_DIVS, M68sz_none, rightop, leftop, Codes);
	    GenInst(M68op_SWAP, M68sz_none, leftop, NULL, Codes);
	    GenInst(M68op_EXT, M68sz_long, leftop, NULL, Codes);
	}
    }
    FreeTemp(rightop);
    return leftop;
}

LocAMVia_t
GenMulDiv(Opcode_t OP, ParseTreeVia_t left,
	  ParseTreeVia_t right, InstListVia_t Codes)
{
    LocAMVia_t                      leftop;
    LocAMVia_t                      rightop;
    /* The possibilities for OP here are MULS and DIVS */
    leftop = GenExpression(left, Codes);
    rightop = GenExpression(right, Codes);
    if (isFloatingType(GetTreeTP(left))) {
	if (gProject->itsOptions->useMC68881) {
	    leftop = DictateTempFloat(leftop, Codes);
	    if (isShortFloatingType(GetTreeTP(left))) {
		switch (OP) {
		case M68op_MULS:
		    GenInst(M68op_FSGLMUL, M68sz_single, rightop, leftop, Codes);
		    break;
		case M68op_DIVS:
		    GenInst(M68op_FSGLDIV, M68sz_single, rightop, leftop, Codes);
		    break;
		default:
		    Gen68Error("Illegal opcode in gen68_muldiv for floats");
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
	} else {
	    leftop = DictateTempFloat(leftop, Codes);
	    rightop = DictateTempFloat(rightop, Codes);
	    switch (OP) {
	    case M68op_MULS:
		GenSANECall(FFEXT + FOMUL, rightop, leftop, Codes);
		break;
	    case M68op_DIVS:
		GenSANECall(FFEXT + FODIV, rightop, leftop, Codes);
		break;
	    }
	}
    } else {
	if (isUnsignedType(GetTreeTP(left))) {
	    leftop = GenMulUnsigned(OP, leftop, rightop, Codes);
	} else {
	    leftop = GenMulSigned(OP, leftop, rightop, Codes);
	}
    }
    FreeTemp(rightop);
    return leftop;
}

LocAMVia_t
GenSubscript(ParseTreeVia_t expr,
	     InstListVia_t Codes)
{
    LocAMVia_t                      result;
    LocAMVia_t                      baseop;
    LocAMVia_t                      subop;
    LocAMVia_t                      multiplier;
    FoldValue_t                     testK;
    baseop = GenExpression(Via(expr)->a, Codes);
    ConstExprValue(Via(expr)->b, &testK);
    if (testK.isK) {
	int                             offset;
	offset = GetTPSize(GetTPBase(GetTreeTP(Via(expr)->a))) * testK.intval;
	baseop = DictateTempAReg(baseop, Codes);
	if (offset) {
	    subop = BuildImmediate(offset, M68sz_long);
	    /* MOTOM Should be ADDA */
	    GenInst(M68op_ADD, M68sz_long, subop, baseop, Codes);
	    FreeTemp(subop);
	}
    } else {
	subop = GenExpression(Via(expr)->b, Codes);
	subop = DictateTempDReg(subop, Codes);
	if (GetTPSize(GetTPBase(GetTreeTP(Via(expr)->a))) != 1) {
	    multiplier = BuildImmediate(GetTPSize(GetTPBase(GetTreeTP(Via(expr)->a))),
					GetLocSZ(subop));
	    subop = GenMulSigned(M68op_MULS, subop, multiplier, Codes);
	    FreeTemp(multiplier);
	}
	if (GetLocSZ(subop) != M68sz_long) {
	    GenInst(M68op_EXT, M68sz_long, subop, NULL, Codes);
	}
	baseop = DictateTempAReg(baseop, Codes);
	/* MOTOM Should be ADDA */
	GenInst(M68op_ADD, M68sz_long, subop, baseop, Codes);
	FreeTemp(subop);
    }
    result = BuildARegIndirect(GetLocAReg(baseop));
    if (isTempReg(baseop)) {
	TempAddrs[GetLocAReg(baseop)].holder = result;
	/* Dangerous move. */
    }
    SetLocSZ(result, M68_TypeSize(GetTreeTP(expr)));
    return result;
}

LocAMVia_t
GenIndirectMember(ParseTreeVia_t expr, InstListVia_t Codes)
{
    LocAMVia_t                      retres;
    LocAMVia_t                      result;
    if (isBitFieldType(GetTreeTP(expr))) {
	result = GenExpression(Via(expr)->a, Codes);
	result = DictateAnyAReg(result, Codes);
	retres = BuildARegDisplaceField(GetLocAReg(result),
					Via(Via(expr)->data.thesymbol)->numbers.structoffset, Via(Via(expr)->data.thesymbol)->Definition.StartEndBits);
	if (isTempReg(result)) {
	    TempAddrs[GetLocAReg(result)].holder = retres;
	    /*
	     * Dangerous operation - this should probably be documented and
	     * moved into its own routine.
	     */
	}
	SetLocSZ(retres, M68sz_long);
	return retres;
    } else {
	result = GenExpression(Via(expr)->a, Codes);
	result = DictateAnyAReg(result, Codes);
	retres = BuildARegDisplace(GetLocAReg(result),
		      Via(Via(expr)->data.thesymbol)->numbers.structoffset);
	SetLocSZ(retres, M68_TypeSize(GetTreeTP(expr)));
	if (isTempReg(result)) {
	    TempAddrs[GetLocAReg(result)].holder = retres;
	    /*
	     * Dangerous operation - this should probably be documented and
	     * moved into its own routine.
	     */
	}
	return retres;
    }
}

LocAMVia_t
GenMember(ParseTreeVia_t expr, InstListVia_t Codes)
{
    LocAMVia_t                      retres;
    LocAMVia_t                      result;
    if (isBitFieldType(GetTreeTP(expr))) {
	result = GenAddrOf(Via(expr)->a, Codes);
	retres = BuildARegDisplaceField(GetLocAReg(result),
					Via(Via(expr)->data.thesymbol)->numbers.structoffset, Via(Via(expr)->data.thesymbol)->Definition.StartEndBits);
	TempAddrs[GetLocAReg(result)].holder = retres;
	/*
	 * Dangerous operation - this should probably be documented and moved
	 * into its own routine.
	 */
	SetLocSZ(retres, M68sz_long);
	return retres;
    } else {
#define MAYBENOT
#ifdef MAYBENOT
	if (Via(Via(expr)->data.thesymbol)->numbers.structoffset) {
	    result = GenAddrOf(Via(expr)->a, Codes);
	    retres = BuildARegDisplace(GetLocAReg(result),
		      Via(Via(expr)->data.thesymbol)->numbers.structoffset);
	} else {
	    retres = GenExpression(Via(expr)->a, Codes);
	    result = NULL;
	}
#else
	result = GenAddrOf(Via(expr)->a, Codes);
	retres = BuildARegDisplace(GetLocAReg(result),
		      Via(Via(expr)->data.thesymbol)->numbers.structoffset);
#endif
	SetLocSZ(retres, M68_TypeSize(GetTreeTP(expr)));
	if (result) {
	    TempAddrs[GetLocAReg(result)].holder = retres;
	    /*
	     * Dangerous operation - this should probably be documented and
	     * moved into its own routine.
	     */
	}
	return retres;
    }
}

LocAMVia_t
GenAddrOf(ParseTreeVia_t expr, InstListVia_t Codes)
{
    LocAMVia_t                      result;
    LocAMVia_t                      tempresult;
    result = GenExpression(expr, Codes);
    FreeTemp(result);
    if (GetLocAM(result) == M68am_Immediate)	/* Ugly kludge. */
	return result;
    tempresult = TempAddrReg(Codes);
    SetLocSZ(tempresult, M68sz_long);	/* This assumes that the size of all
					 * pointers is 4. */
    GenLoadEA(result, tempresult, Codes);
    return tempresult;
}

LocAMVia_t
GenDeref(ParseTreeVia_t expr, InstListVia_t Codes)
{
    LocAMVia_t                      result;
    LocAMVia_t                      retres;
    result = GenExpression(Via(expr)->a, Codes);
    result = DictateTempAReg(result, Codes);
    retres = BuildARegIndirect(GetLocAReg(result));
    TempAddrs[GetLocAReg(result)].holder = retres;
    result = retres;
    SetLocSZ(result, M68_TypeSize(GetTreeTP(expr)));
    return result;
}

LocAMVia_t
GenBitNegate(ParseTreeVia_t expr, InstListVia_t Codes)
{
    LocAMVia_t                      result;
    result = GenExpression(expr, Codes);
    result = DictateTempDReg(result, Codes);
    GenInst(M68op_NOT, GetTPSize(GetTreeTP(expr)), result, NULL, Codes);
    return result;
}

LocAMVia_t
GenNegate(ParseTreeVia_t expr, InstListVia_t Codes)
{
    LocAMVia_t                      result;
    result = GenExpression(expr, Codes);
    if (isFloatingType(GetTreeTP(expr))) {
	result = DictateTempFloat(result, Codes);
	GenInst(M68op_FNEG, GetLocSZ(result), result, NULL, Codes);
    } else {
	result = DictateTempDReg(result, Codes);
	GenInst(M68op_NEG, GetTPSize(GetTreeTP(expr)), result, NULL, Codes);
    }
    return result;
}

LocAMVia_t
GenIdentifier(ParseTreeVia_t expr, InstListVia_t Codes)
/*
 * General identifiers are handled here.
 */
{
    LocAMVia_t                      result;
    result = Via(Via(expr)->data.thesymbol)->M68kDef.Loc;
    if (!result) {
		Gen68Error("Symbol with no location");
    }
    if (GetLocAM(result) == M68am_LargeGlobal) {
		LocAMVia_t                      temp;
		LocAMVia_t                      temp2;
		temp = TempAddrReg(Codes);
		Genmove(M68sz_long, BuildARegDirect(5), temp,
			Codes);
		GenInst(M68op_ADD, M68sz_long, result, temp, Codes);
		temp2 = BuildARegIndirect(GetLocAReg(temp));
		TempAddrs[GetLocAReg(temp)].holder = temp2;
		result = temp2;
    }
    return result;
}

LocAMVia_t
GenStringLit(ParseTreeVia_t expr, InstListVia_t Codes)
{
    LocAMVia_t                      result;
    LocAMVia_t                      temp;
    result = Via(Via(expr)->data.SLit)->M68kDef.Loc;
    temp = TempAddrReg(Codes);
    GenLoadEA(result, temp, Codes);
    result = temp;
    return result;
}

LocAMVia_t
GenExpression(ParseTreeVia_t expr, InstListVia_t Codes)
/*
 * This is the general routine to call to generate code for any expression or
 * statement tree.
 */
{
    LocAMVia_t                      result;
    result = NULL;
    if (expr) {
	switch (Via(expr)->kind) {
	case PTF_array_subscript:
	    result = GenSubscript(expr, Codes);
	    break;
	case PTF_function_call:
	    result = GenFunctionCall(expr, Codes);
	    break;
	case PTF_postincrement:
	    result = GenPostInc(Via(expr)->a, Codes);
	    break;
	case PTF_postdecrement:
	    result = GenPostDec(Via(expr)->a, Codes);
	    break;
	case PTF_argument_list:
	    /* This should never happen */
	    break;
	case PTF_preincrement:
	    result = GenPreInc(Via(expr)->a, Codes);
	    break;
	case PTF_predecrement:
	    result = GenPreDec(Via(expr)->a, Codes);
	    break;
	case PTF_gen_addrof:	/* Should this be this way ?? */
	case PTF_address_of:
	    result = GenAddrOf(Via(expr)->a, Codes);
	    break;
	case PTF_deref:
	    result = GenDeref(expr, Codes);
	    break;
	case PTF_unary_plus:
	    result = GenExpression(Via(expr)->a, Codes);
	    break;
	case PTF_unary_minus:
	    result = GenNegate(Via(expr)->a, Codes);
	    break;
	case PTF_bitwise_neg:
	    result = GenBitNegate(Via(expr)->a, Codes);
	    break;
	case PTF_logical_neg:
	    result = GenLogicalNegate(Via(expr)->a, Codes);
	    break;
	case PTF_sizeof:
	    result = BuildImmediate(GetTPSize(Via(expr)->data.TP), GetTPSize(GetTreeTP(expr)));
	    break;
	case PTF_enumconstant:
	    result = BuildImmediate(Via(Via(expr)->data.thesymbol)->numbers.EnumVal,
				    GetTPSize(GetTreeTP(expr)));
	    break;
	case PTF_multiply:
	    result = GenMulDiv(M68op_MULS, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_divide:
	    result = GenMulDiv(M68op_DIVS, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_modulo:
	    result = GenModulo(Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_typechange:
	    result = GenTypeChange(expr, Codes);
	    break;
	case PTF_add:
	    result = GenTwoOp(M68op_ADD, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_subtract:
	    result = GenTwoOp(M68op_SUB, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_shift_left:
	    result = GenbitTwoOp(M68op_ASL, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_shift_right:
	    result = GenbitTwoOp(M68op_ASR, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_lessthan:
	    result = GenRelational(M68op_BLT, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_greaterthan:
	    result = GenRelational(M68op_BGT, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_lessthaneq:
	    result = GenRelational(M68op_BLE, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_greaterthaneq:
	    result = GenRelational(M68op_BGE, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_equal:
	    result = GenRelational(M68op_BEQ, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_notequal:
	    result = GenRelational(M68op_BNE, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_bitwise_and:
	    result = GenbitTwoOp(M68op_AND, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_bitwise_xor:
	    result = GenbitTwoOp(M68op_EOR, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_bitwise_or:
	    result = GenbitTwoOp(M68op_OR, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_logical_and:
	    result = GenLogicalAnd(Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_logical_or:
	    result = GenLogicalOr(Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_ternary:
	    result = GenTernary(Via(expr)->a, Via(expr)->b, Via(expr)->c, Codes);
	    break;
	case PTF_assign:
	    result = GenAssign(Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_mulassign:
	    result = GenMulDivAssign(M68op_MULS, Via(expr)->a, Via(expr)->b,
				     Codes);
	    break;
	case PTF_divassign:
	    result = GenMulDivAssign(M68op_DIVS, Via(expr)->a, Via(expr)->b,
				     Codes);
	    break;
	case PTF_modassign:
	    result = GenModAssign(Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_addassign:
	    result = GenOPAssign(M68op_ADD, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_subassign:
	    result = GenOPAssign(M68op_SUB, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_leftassign:
	    result = GenOPAssignDREG(M68op_ASL, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_rightassign:
	    result = GenOPAssignDREG(M68op_ASR, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_andassign:
	    result = GenOPAssign(M68op_AND, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_xorassign:
	    result = GenOPAssignDREG(M68op_EOR, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_orassign:
	    result = GenOPAssign(M68op_OR, Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_commas:
	    GenExpression(Via(expr)->a, Codes);
	    result = GenExpression(Via(expr)->b, Codes);
	    break;
	case PTF_multi_initializer:
	    result = GenExpression(Via(expr)->a, Codes);
	    break;
	case PTF_initializer_list:
	    result = NULL;
	    Gen68Error("Multiple initializers are not allowed for auto variables.");
	    break;
	case PTF_exprstmt:
	    FreeAllRegs();
	    /*
	     * TODO It would be nice to be able to stick annotations on ALL
	     * kinds of statements, but most will be too many lines long, and
	     * we cannot control when comments begin or end.
	     */
	    DiscardedFunctionCall = 0;
	    if (Via(expr)->a) {
		if (Via(Via(expr)->a)->kind == PTF_function_call) {
		    DiscardedFunctionCall = 1;
		}
	    }
	    result = GenExpression(Via(expr)->a, Codes);
	    FreeTemp(result);
	    break;
	case PTF_emptystmt:
	    break;
	case PTF_switchcase_stmt:
	    GenSwitchCase(expr, Codes);
	    break;
	case PTF_switchdefault_stmt:
	    GenSwitchDefault(expr, Codes);
	    break;
	case PTF_compound_stmt:
	    GenCompound(expr, Codes);
	    break;
	case PTF_stmt_list:
	    GenExpression(Via(expr)->b, Codes);
	    GenExpression(Via(expr)->a, Codes);
	    break;
	case PTF_ifthenelse_stmt:
	    GenIfThenElse(Via(expr)->a, Via(expr)->b, Via(expr)->c, Codes);
	    break;
	case PTF_switch_stmt:
	    GenSWITCH(Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_while_stmt:
	    GenWhile(Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_dowhile_stmt:
	    GenDoWhile(Via(expr)->a, Via(expr)->b, Codes);
	    break;
	case PTF_for_stmt:
	    Genforstmt(Via(expr)->a, Via(expr)->b, Via(expr)->c, GetTreeFour(expr), Codes);
	    break;
	case PTF_continue_stmt:
	    GenInst(M68op_BRA, M68sz_none,
		    BuildLabelLoc(ContinueLabel), NULL, Codes);
	    break;
	case PTF_break_stmt:
	    GenInst(M68op_BRA, M68sz_none,
		    BuildLabelLoc(BreakLabel), NULL, Codes);
	    break;
	case PTF_return_stmt:
	    GenReturn(Via(expr)->a, Codes);
	    break;
	case PTF_identifier:
	    result = GenIdentifier(expr, Codes);
	    break;
	case PTF_intconstant:
	    result = BuildImmediate(Via(expr)->data.number, GetTPSize(GetTreeTP(expr)));
	    break;
	case PTF_floatconstant:
	    result = Via(Via(expr)->data.FLit)->Loc;
	    /*
	     * QQQQ We should not have to do lea stuff here, because we
	     * essentially ALWAYS PEA this Loc , right ?
	     */
	    break;
	case PTF_string_literal:
	    result = GenStringLit(expr, Codes);
	    if (!result) {
		Gen68Error("String lit with no location");
	    }
	    break;
	case PTF_struct_member:
	    result = GenMember(expr, Codes);
	    break;
	case PTF_struct_indirect_member:
	    result = GenIndirectMember(expr, Codes);
	    break;
	case PTF_labelled_stmt:
	    GenLabelledStmt(expr, Codes);
	    break;
	case PTF_goto_stmt:
	    GenGotoStmt(expr, Codes);
	    break;
	case PTF_NOP:
	    result = NULL;
	    break;
	case PTF_asm_stmt:
	    AddInstList(Via(expr)->data.AsmCodes, Codes);
	    break;
	default:
	    break;
	}
    }
    return result;
}
