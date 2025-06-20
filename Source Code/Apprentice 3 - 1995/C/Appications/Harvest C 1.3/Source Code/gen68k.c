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
 * This file contains routines for 68k code generation.  The result is a set of
 * data structures representing 68k instructions and data. The routines here
 * do not output assembly source.
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

#pragma segment CodeGenMisc

extern LabSYMVia_t NextFuncSegment;

/*
 * This file and its friends contain routines used for generating 68000
 * assembly code from the symbol table / parse tree.  These routines convert
 * data structures to data structures.  The result is a data structure
 * representing 68000 instructions.  It is the job of another module to
 * handle the outputting of these data structures as assembler source or
 * linker files or whatever is desired.
 */

/*
 * The routines used for generating 68000 instructions for the various
 * statements and expressions will resemble the parser.  The important data
 * structure with respect to the expressions is the LocAM.  This data
 * structure simply stores an addressing mode which specifies where the
 * operand is "currently" located.  A general routine to generate code for an
 * expression will accept the parse tree as an argument and generate 68000
 * instruction records into the current list of instructions, and return the
 * LocAM data structure, specifying where the result is located. There will
 * be roughly one routine for each type of expression or statement, just as
 * in the parser, and each routine will potentially call several others,
 * recursively calling itself, as in the parser.  _Statements_ need not
 * return an addressing mode, as they have no result value.  A 68000
 * instruction record must store the opcode, both operands, size, the label
 * for that instruction. An operand stores an addressing mode and its size.
 * In the process of generating expressions, temporary registers will be
 * needed.
 */

/*
 * During the code generation of expressions, we often want to ensure that
 * some value is in a register, so that we may do operations on it which are
 * restricted to register modes.  Also, we want values to be placed in
 * temporary places so that intermediate value will not clobber variables.
 * For example, x = a + b, where all three are locals on the stack frame.  We
 * do not want to simply add a to b and move the result to x, because this
 * has the side effect of changing b.  So, we ensure that b is moved to a
 * register before doing the addition.  The problem is that there are a
 * limited number of temporary registers. What happens when we request a
 * temporary register, and they are all full ? Well, we spill the value in
 * some register onto the stack, and give the register to the value which
 * requested it.  So, the problem now is : What happens when we want the
 * value that WAS in the register ?  Well, it must be retrieved from the
 * stack.  When we retrieve something from the stack, what happens if the
 * register we are retrieving it into is currently full ?  When we retrieve
 * something from the stack, what happens if its not on top of the stack ?
 * Are there situations where this can come up ?
 */

InstVia_t
RawInstruction(void)
{
    InstVia_t                       raw;
    raw = Ealloc(sizeof(Inst_t));
    if (raw) {
	Via(raw)->OP = 0;
	Via(raw)->left = NULL;
	Via(raw)->right = NULL;
	Via(raw)->SZ = M68sz_none;
	Via(raw)->prev = NULL;
	Via(raw)->Address = -1;
	Via(raw)->InstSize = -1;
	Via(raw)->Bytes[0] = 0;
	Via(raw)->next = NULL;
    }
    return raw;
}

InstListVia_t
RawInstructionList(void)
{
    InstListVia_t                   raw;
    raw = Ealloc(sizeof(InstList_t));
    if (raw) {
	Via(raw)->head = NULL;
	Via(raw)->tail = NULL;
	Via(raw)->count = 0;
	Via(raw)->PendingLabel = NULL;
    }
    return raw;
}

enum Size68
M68_TypeSize(TypeRecordVia_t TP)
/*
 * Returns the size of a type, for the purpose of the size field of an
 * instruction.  The routine is really only necessary because of the 68881
 * code generator.
 */
{
    if (isFloatingType(TP)) {
	if (gProject->itsOptions->useMC68881) {
	    if (GetTPKind(TP) == TRC_float) {
		return M68sz_single;
	    } else if (GetTPKind(TP) == TRC_double) {
		return M68sz_double;
	    } else {
		return M68sz_extended;
	    }
	} else {
	    return GetTPSize(TP);
	}
    } else if (isPointerType(TP)) {
	return M68sz_long;
    } else if (isArrayType(TP)) {
	return M68sz_long;
    } else if (isFunctionType(TP)) {
	/* Should probably generate a warning here. */
	return M68sz_long;
    } else {
	return GetTPSize(TP);
    }
}

void
GenLoadEA(LocAMVia_t loc, LocAMVia_t dst, InstListVia_t Codes)
/*
 * This routine is called when we want to LEA something, generally when we
 * want the address of something.
 */
{
    assert(GetLocAM(loc) != M68am_DReg);
    if (GetLocAM(loc) == M68am_ARegIndirect) {
	GenInst(M68op_MOVE, M68sz_long, BuildARegDirect(GetLocAReg(loc)),
		dst, Codes);
    } else {
	GenInst(M68op_LEA, M68sz_none, loc, dst, Codes);
    }
}

void
GenPushEA(LocAMVia_t loc, InstListVia_t Codes)
/*
 * This routine is called when we want to PEA something, generally when we
 * want the address of something.
 */
{
    if (GetLocAM(loc) == M68am_ARegIndirect) {
		GenInst(M68op_MOVE, M68sz_long, BuildARegDirect(GetLocAReg(loc)),
			BuildARegPreDec(7), Codes);
    } else {
		GenInst(M68op_PEA, M68sz_none, loc, NULL, Codes);
    }
}

LocAMVia_t
GenSANECall(unsigned short Cmd, LocAMVia_t src, LocAMVia_t dst, InstListVia_t Codes)
/* This routine generates a call to SANE. */
{
    /*
     * We  make sure both operands are in spill slots, and pea of both, src
     * first.  Then we push the Cmd word, and generate _FP68K
     */

    if (src)
	GenPushEA(src, Codes);
    if (dst)
	GenPushEA(dst, Codes);
    GenPush(BuildImmediate(Cmd, M68sz_word), Codes);
    GenInst(M68op_FP68K, M68sz_none, NULL, NULL, Codes);
    return dst;
}

void
Genmoverange(int size, LocAMVia_t src, LocAMVia_t dst, InstListVia_t Codes)
{
    /*
     * This routine handles a move of any even number of bytes from one
     * location to another.  It is used for struct assignment, moving of
     * doubles, etc...
     */
    LocAMVia_t                      fromA = NULL;
    LocAMVia_t                      toA = NULL;
    LocAMVia_t                      fromTemp = NULL;
    LocAMVia_t                      toTemp = NULL;
    int                             dstoff = 0;
    int                             srcoff = 0;
    int                             dsttmp = 0;
    int                             srctmp = 0;
    int                             ndx = 0;
    if (GetLocAM(dst) == M68am_ARegPreDec) {
	/* This is usually a stack push. */
	fromA = TempAddrReg(Codes);
	SetLocAM(fromA, M68am_ARegDirect);
	if (GetLocAM(src) == M68am_DReg) {
	    Gen68Error("Can't LEA a D reg");
	}
	GenLoadEA(src, fromA, Codes);
	ndx = size;
	while (ndx) {
	    if (ndx == 2) {
		fromTemp = BuildARegDisplace(GetLocAReg(fromA), ndx - 2);
		GenInst(M68op_MOVE, M68sz_word, fromTemp, dst, Codes);
		ndx -= 2;
	    } else {
		fromTemp = BuildARegDisplace(GetLocAReg(fromA), ndx - 4);
		GenInst(M68op_MOVE, M68sz_long, fromTemp, dst, Codes);
		ndx -= 4;
	    }
	}
	FreeTemp(fromA);
    } else {
	srctmp = dsttmp = 0;
	if (GetLocAM(src) == M68am_ARegDisplace) {
	    fromA = BuildARegDisplace(GetLocAReg(src), GetLocConstant(src));
	    srcoff = GetLocConstant(src);
	} else if (GetLocAM(src) == M68am_ARegIndirect) {
	    fromA = BuildARegDisplace(GetLocAReg(src), 0);
	    srcoff = 0;
	} else {
	    srctmp = 1;
	    fromA = TempAddrReg(Codes);
	    SetLocAM(fromA, M68am_ARegDirect);
	    GenLoadEA(src, fromA, Codes);
	    srcoff = 0;
	}
	if (GetLocAM(dst) == M68am_ARegPostInc) {
	    toTemp = dst;
	} else if (GetLocAM(dst) == M68am_ARegDisplace) {
	    toTemp = BuildARegDisplace(GetLocAReg(dst), GetLocConstant(dst));
	    dstoff = GetLocConstant(dst);
	} else if (GetLocAM(dst) == M68am_ARegIndirect) {
	    toTemp = BuildARegDisplace(GetLocAReg(dst), 0);
	    dstoff = 0;
	} else {
	    dsttmp = 1;
	    toA = TempAddrReg(Codes);
	    SetLocAM(toA, M68am_ARegDirect);
	    GenLoadEA(dst, toA, Codes);
	    toTemp = BuildARegPostInc(GetLocAReg(toA));
	    dstoff = 0;
	}
	ndx = 0;
	while (ndx < size) {
	    fromTemp = BuildARegDisplace(GetLocAReg(fromA), srcoff);
	    if (GetLocAM(toTemp) == M68am_ARegDisplace) {
		toTemp = BuildARegDisplace(GetLocAReg(toTemp), dstoff);
	    }
	    if ((size - ndx) == 2) {
		GenInst(M68op_MOVE, M68sz_word, fromTemp, toTemp, Codes);
		ndx += 2;
		srcoff += 2;
		dstoff += 2;
	    } else {
		GenInst(M68op_MOVE, M68sz_long, fromTemp, toTemp, Codes);
		ndx += 4;
		srcoff += 4;
		dstoff += 4;
	    }
	}
	if (srctmp) {
	    FreeTemp(fromA);
	}
	if (dsttmp) {
	    FreeTemp(toA);
	}
    }
}

int
is68881SZ(enum Size68 size)
{
    switch (size) {
	case M68sz_single:
	case M68sz_double:
	case M68sz_extended:
	return 1;
	break;
    default:
	return 0;
	break;
    }
}

void
Genmove(enum Size68 size, LocAMVia_t src, LocAMVia_t dst, InstListVia_t Codes)
{
    /*
     * This is a general purpose move routine, to move any operand to any
     * other operand.
     */

    if (size == M68sz_none) {
	Gen68Error("Missing size in gen68_move");
    }
    if ((GetLocAM(src) == M68am_FReg) || (GetLocAM(dst) == M68am_FReg)) {
	GenInst(M68op_FMOVE, size, src, dst, Codes);
    } else {
	if (is68881SZ(size)) {
	    if (SizeOfFloat == SizeOfDouble) {
		Genmoverange(SizeOfDouble, src, dst, Codes);
	    } else {
		if (size == M68sz_single) {
		    GenInst(M68op_MOVE, M68sz_long, src, dst, Codes);
		} else if (size == M68sz_double) {
		    Genmoverange(SizeOfDouble, src, dst, Codes);
		} else {
		    Genmoverange(SizeOfLongDouble, src, dst, Codes);
		}
	    }
	} else if ((LocIsFloat(src)) || (LocIsFloat(dst))) {
	    if (GetLocSZ(src) == GetLocSZ(dst)) {
		Genmoverange(GetLocSZ(dst), src, dst, Codes);
	    } else {		/* different sizes (conversion) */
		LocAMVia_t                      ftemp;
		if (GetLocSZ(src) == SizeOfFloat) {
		    if (GetLocSZ(dst) == SizeOfDouble) {
			ftemp = TempFloatReg(Codes);
			GenSANECall(FFSGL + FOZ2X, src, ftemp, Codes);
			GenSANECall(FFDBL + FOX2Z, ftemp, dst, Codes);
		    } else {	/* extended */
			GenSANECall(FFSGL + FOZ2X, src, dst, Codes);
		    }
		} else if (GetLocSZ(src) == SizeOfDouble) {
		    if (GetLocSZ(dst) == SizeOfFloat) {
			ftemp = TempFloatReg(Codes);
			GenSANECall(FFDBL + FOZ2X, src, ftemp, Codes);
			GenSANECall(FFSGL + FOX2Z, ftemp, dst, Codes);
		    } else {	/* extended */
			GenSANECall(FFDBL + FOZ2X, src, dst, Codes);
		    }
		} else {	/* extended */
		    if (GetLocSZ(dst) == SizeOfFloat) {
			GenSANECall(FFSGL + FOX2Z, src, dst, Codes);
		    } else {	/* double */
			GenSANECall(FFDBL + FOX2Z, src, dst, Codes);
		    }
		}
	    }
	} else if (size > 4) {
	    Genmoverange(size, src, dst, Codes);
	} else {
	    GenInst(M68op_MOVE, size, src, dst, Codes);
	}
    }
}

SpillSlotVia_t
GetLocalSlot(TypeRecordVia_t TP)
/*
 * Gets a spill slot specifically designed to be used for a local variable.
 */
{
    SpillSlotVia_t                  result;
    if (isArrayType(TP)) {
	result = GetSpillSlot(GetTPSize(TP));
    } else {
	result = GetSpillSlot(M68_TypeSize(TP));
    }
    Via(result)->local = 1;
    return result;
}

void
AddInstruction(InstVia_t inst, InstListVia_t Codes)
/*
 * Adds an instruction to the instruction list passed.  If the instruction is
 * a label, then that label is set as the pending label for that list.  If
 * there is a pending label for the list, that label is assigned to point to
 * the given instruction, even if that instruction is yet another label.
 */
{
    if (Via(Codes)->PendingLabel) {
	Via(Via(Codes)->PendingLabel)->M68kDef.where = inst;
	Via(Codes)->PendingLabel = NULL;
    }
    if (Via(inst)->OP == M68op_DATALABEL) {
	Via(Codes)->PendingLabel = GetLocLabel(Via(inst)->left);
    }
    if (Via(inst)->OP == M68op_CODELABEL) {
	Via(Codes)->PendingLabel = GetLocLabel(Via(inst)->left);
    }
    if (Via(inst)->OP == M68op_LABEL) {
	Via(Codes)->PendingLabel = GetLocLabel(Via(inst)->left);
    }
    if (Via(inst)->OP == M68op_STRINGLABEL) {
	Via(Codes)->PendingLabel = GetLocLabel(Via(inst)->left);
    }
    if (Via(Codes)->tail) {
	Via(Via(Codes)->tail)->next = inst;
	Via(inst)->prev = Via(Codes)->tail;
	Via(Codes)->tail = inst;
	Via(Codes)->count++;
    } else {
	Via(Codes)->head = Via(Codes)->tail = inst;
	Via(Codes)->count = 1;
    }
}

InstVia_t
InsertInstruction(Opcode_t OP, enum Size68 SZ, LocAMVia_t left, LocAMVia_t right, InstVia_t after)
{
    /*
     * Generate an instruction record and insert it after after.
     */
    InstVia_t                       inst;
    inst = RawInstruction();
    if (inst) {
	Via(inst)->OP = OP;
	Via(inst)->SZ = SZ;
	Via(inst)->left = left;
	Via(inst)->right = right;
	Via(inst)->prev = after;
	Via(inst)->next = Via(after)->next;
	if (Via(after)->next) {
	    Via(Via(after)->next)->prev = inst;
	}
	Via(after)->next = inst;
    }
    return inst;
}

InstVia_t
GenInst(Opcode_t OP, enum Size68 SZ, LocAMVia_t left, LocAMVia_t right, InstListVia_t Codes)
{
    /*
     * Generate an instruction record and insert it into Codes. This routine
     * automatically checks to make sure that its operands are valid (ie -
     * not spilled)
     */
    register InstVia_t              inst;
    if (left) {
	if (GetLocStatus(left) == M68os_spilled) {
	    left = ValidateReg(left, Codes);
	}
    }
    if (right) {
	assert(left);
	if (GetLocStatus(right) == M68os_spilled) {
	    right = ValidateReg(right, Codes);
	}
    }
    if (SZ == 0) {
	Gen68Error("SZ = 0 in gen68_inst");
    }
    if (SZ > 4) {
	Gen68Error("SZ > 4 in gen68_inst");
    }
    if (left) {
	if (right) {
	    if ((GetLocAM(left) == M68am_FReg) && (GetLocAM(right) == M68am_FReg)) {
		SZ = M68sz_extended;
	    }
	} else {
	    if ((GetLocAM(left) == M68am_FReg)) {
		SZ = M68sz_extended;
	    }
	}
    }
    inst = RawInstruction();
    if (inst) {
	Via(inst)->OP = OP;
	Via(inst)->SZ = SZ;
	Via(inst)->left = left;
	Via(inst)->right = right;
	AddInstruction(inst, Codes);
    }
    return inst;
}

void
AddInstList(InstListVia_t list, InstListVia_t Codes)
/* Adds all the contents of one instruction list to another. */
{
    InstVia_t                       inst;
    if (list) {
	inst = Via(list)->head;
	while (inst) {
	    AddInstruction(inst, Codes);
	    inst = Via(inst)->next;
	}
    }
}

void
GenPush(LocAMVia_t loc, InstListVia_t Codes)
/*
 * Generates a push instruction, pushing the arg loc onto the stack.
 */
{
    LocAMVia_t                      stkpush;
    stkpush = RawLocation();
    SetLocAM(stkpush, M68am_ARegPreDec);
    SetLocAReg(stkpush, 7);
    /*
     * This special case should probably be unnecessary if the sizing code
     * for doubles were really handled right.
     */
    if (GetLocSZ(loc) == M68sz_extended) {
		Genmove(M68sz_double, loc, stkpush, Codes);
    } else if (GetLocSZ(loc) == 1) {
		/* Bytes are really pushed as words. */
		/* This code should be unnecessary */
		loc = DictateTempDReg(loc, Codes);
		GenInst(M68op_EXT, M68sz_word, loc, NULL, Codes);
		Genmove(M68sz_word, loc, stkpush, Codes);
		FreeTemp(loc);
    } else if (GetLocAM(loc) == M68am_FSANEtemp) {
		/*
		 * This should be pushed as a double, or a float, but not as
		 * extended.
		 */
		SpillSlotVia_t                  slot;
		LocAMVia_t                      lop;
		if (GetLocSZ(loc) == 4) {
		    slot = GetSpillSlot(4);
		    lop = Via(slot)->SlotLoc;
		    GenSANECall(FFSGL + FOX2Z, loc, lop, Codes);
		    GenInst(M68op_MOVE, M68sz_long, lop, stkpush, Codes);
		    FreeTemp(lop);
		} else {
		    slot = GetSpillSlot(12);
		    lop = Via(slot)->SlotLoc;
		    GenSANECall(FFDBL + FOX2Z, loc, lop, Codes);
		    GenInst(M68op_MOVE, M68sz_long, BuildARegDisplace(GetLocAReg(lop), GetLocConstant(lop) + 4), stkpush, Codes);
		    GenInst(M68op_MOVE, M68sz_long, lop, stkpush, Codes);
		    FreeTemp(lop);
		}
    } else {
		Genmove(GetLocSZ(loc), loc, stkpush, Codes);
    }
}

void
GenPascalPush(LocAMVia_t loc, InstListVia_t Codes)
/*
 * Generates a push instruction, pushing the arg loc onto the stack.
 */
{
    LocAMVia_t                      stkpush;
    stkpush = RawLocation();
    SetLocAM(stkpush, M68am_ARegPreDec);
    SetLocAReg(stkpush, 7);
    /*
     * This special case should probably be unnecessary if the sizing code
     * for doubles were really handled right.
     */
    if (GetLocSZ(loc) == M68sz_extended) {
		Genmove(M68sz_double, loc, stkpush, Codes);
    } else if (GetLocSZ(loc) == 1) {
		Genmove(M68sz_byte, loc, stkpush, Codes);
    } else if (GetLocAM(loc) == M68am_FSANEtemp) {
		/*
		 * This should be pushed as a double, or a float, but not as
		 * extended.
		 */
		SpillSlotVia_t                  slot;
		LocAMVia_t                      lop;
		if (GetLocSZ(loc) == 4) {
		    slot = GetSpillSlot(4);
		    lop = Via(slot)->SlotLoc;
		    GenSANECall(FFSGL + FOX2Z, loc, lop, Codes);
		    GenInst(M68op_MOVE, M68sz_long, lop, stkpush, Codes);
		    FreeTemp(lop);
		} else {
		    slot = GetSpillSlot(12);
		    lop = Via(slot)->SlotLoc;
		    GenSANECall(FFDBL + FOX2Z, loc, lop, Codes);
		    GenInst(M68op_MOVE, M68sz_long, BuildARegDisplace(GetLocAReg(lop), GetLocConstant(lop) + 4), stkpush, Codes);
		    GenInst(M68op_MOVE, M68sz_long, lop, stkpush, Codes);
		    FreeTemp(lop);
		}
    } else {
		Genmove(GetLocSZ(loc), loc, stkpush, Codes);
    }
}

void
GenCodeLabel(LabSYMVia_t labsym, InstListVia_t Codes)
/* Generates a label marking the beginning of a code module */
{
    GenInst(M68op_MPWSEG, M68sz_none, BuildLabelLoc(NextFuncSegment), NULL, Codes);
    GenInst(M68op_CODELABEL, M68sz_none, BuildLabelLoc(labsym), NULL, Codes);
}

void
GenDataLabel(LabSYMVia_t labsym, InstListVia_t Codes)
/* Generates a label marking the beginning of a data module */
{
    GenInst(M68op_DATALABEL, M68sz_none,
	    BuildLabelLoc(labsym), NULL, Codes);
}

void
GenStringLabel(LabSYMVia_t labsym, InstListVia_t Codes)
/* Generates a label marking the beginning of a string lit */
{
    GenInst(M68op_STRINGLABEL, M68sz_none,
	    BuildLabelLoc(labsym), NULL, Codes);
}

void
GenLabel(LabSYMVia_t labsym, InstListVia_t Codes)
/* Shorter call for generating a label instruction. */
{
    GenInst(M68op_LABEL, M68sz_none,
	    BuildLabelLoc(labsym), NULL, Codes);
}

void
ShiftRight(LocAMVia_t loc, int shifts, InstListVia_t Codes)
/*
 * Used in bit fields, generates code to shift the given loc to the right
 */
{
    if (shifts > 0) {
	int                             leftover;
	leftover = shifts;
	while (leftover) {
	    if (leftover > 7) {
		GenInst(M68op_LSR, M68sz_long, BuildImmediate(7, M68sz_long), loc, Codes);
		leftover -= 7;
	    } else {
		GenInst(M68op_LSR, M68sz_long, BuildImmediate(leftover, M68sz_long), loc, Codes);
		leftover = 0;
	    }
	}
    }
}

void
ShiftLeft(LocAMVia_t loc, int shifts, InstListVia_t Codes)
/*
 * Used in bit fields, generates code to shift the given loc to the left
 */
{
    if (shifts > 0) {
	int                             leftover;
	leftover = shifts;
	while (leftover) {
	    if (leftover > 7) {
		GenInst(M68op_LSL, M68sz_long, BuildImmediate(7, M68sz_long), loc, Codes);
		leftover -= 7;
	    } else {
		GenInst(M68op_LSL, M68sz_long, BuildImmediate(leftover, M68sz_long), loc, Codes);
		leftover = 0;
	    }
	}
    }
}

unsigned long
MakeTheMask(int lo, int hi)
{
    /* Construct a mask - used in generating code for bitfields */
    unsigned long                   bitmask;
    int                             ndx;
    bitmask = 0;
    ndx = lo;
    while (ndx <= hi) {
	bitmask |= ((unsigned long) 1) << ndx;
	ndx++;
    }
    return bitmask;
}

unsigned long
MakeMask(LocAMVia_t loc)
/* Used in generating code for bitfields */
{
    return MakeTheMask(GetLocFieldBits(loc).StartBit, GetLocFieldBits(loc).EndBit);
}

void
GenDeConflictize(LocAMVia_t * a, LocAMVia_t * b, InstListVia_t Codes)
{
    /*
     * Given two locs, modify them as necessary to ensure that they are not
     * in conflict.  In other words, ensure that the register allocator did
     * not assign the same register to both.  If so, one will be moved.
     */
    LocAMVia_t                      c;
    LocAMVia_t                      d;
    LocAMVia_t                      newhome;
    if (a && b) {
	c = *a;
	d = *b;
	if (c && d) {
	    if (c == d)
		return;
	    if (!(isTempReg(c) && isTempReg(d)))
		return;
	    if (GetLocAM(c) == M68am_DReg) {
		if (GetLocAM(d) == M68am_DReg) {
		    if (GetLocDReg(c) == GetLocDReg(d)) {
			/*
			 * Conflict detected.  One of these if spilled - we
			 * move the other one to another register.
			 */
			if (GetLocStatus(d) == M68os_valid) {
			    newhome = TempDataReg(Codes);
			    Genmove(M68sz_long, d, newhome, Codes);
			    SetLocSZ(newhome, GetLocSZ(d));
			    FreeTemp(d);
			    *b = newhome;
			    return;
			} else {
			    newhome = TempDataReg(Codes);
			    Genmove(M68sz_long, c, newhome, Codes);
			    SetLocSZ(newhome, GetLocSZ(c));
			    FreeTemp(c);
			    *a = newhome;
			    return;
			}
		    } else
			return;
		} else
		    return;
	    } else if (isARegMode(c)) {
		if (isARegMode(d)) {
		    if (GetLocAReg(c) == GetLocAReg(d)) {
			if (GetLocStatus(c) == M68os_valid) {
			    newhome = TempAddrReg(Codes);
			    SetLocAM(newhome, GetLocAM(c));
			    SetLocSZ(newhome, GetLocSZ(c));
			    Genmove(M68sz_long, BuildARegDirect(GetLocAReg(c)),
			       BuildARegDirect(GetLocAReg(newhome)), Codes);
			    FreeTemp(c);
			    *a = newhome;
			    return;
			} else {
			    newhome = TempAddrReg(Codes);
			    SetLocAM(newhome, GetLocAM(d));
			    SetLocSZ(newhome, GetLocSZ(d));
			    Genmove(M68sz_long, BuildARegDirect(GetLocAReg(d)),
			       BuildARegDirect(GetLocAReg(newhome)), Codes);
			    FreeTemp(d);
			    *b = newhome;
			    return;
			}
		    } else
			return;
		} else
		    return;
	    } else
		return;
	}
    }
}

LocAMVia_t
Slotify(LocAMVia_t loc, InstListVia_t Codes)
/* Given a loc, move it to a spill slot */
{
    SpillSlotVia_t                  slot;
    LocAMVia_t                      result;
    slot = GetSpillSlot(GetLocSZ(loc));
    result = Via(slot)->SlotLoc;
    Genmove(GetLocSZ(result), loc, result, Codes);
    FreeTemp(loc);
    return result;
}

int
PushBackwards(ParseTreeVia_t arg, ParamRecVia_t Param, int *pcount, InstListVia_t Codes)
{
/* This function is used for calling pascal functions */
    int                             result;
    LocAMVia_t                      argop;
    int                             done;
    result = 0;
    done = 0;
    if (arg) {
	if (Via(arg)->kind == PTF_argument_list) {
	    result = PushBackwards(Via(arg)->b, Param, pcount, Codes);
	    argop = GenExpression(Via(arg)->a, Codes);
	    if (!gProject->itsOptions->useMC68881)
			SetLocSZ(argop, GetTPSize(GetTreeTP(Via(arg)->a)));
	    if (Param) {
			if (*pcount < Via(Param)->argcount) {
			    GenInst(M68op_MOVE, GetLocSZ(argop), argop, ParamLoc(Via(Param)->args[*pcount]), Codes);
			    (*pcount)++;
			} else
			    GenPascalPush(argop, Codes);
	    } else
			GenPascalPush(argop, Codes);
	    FreeTemp(argop);
	    result += GetTPSize(GetTreeTP(Via(arg)->a));
	    if (GetLocSZ(argop) == M68sz_byte) {
			/* bytes are really pushed as words. */
			result++;
	    }
	} else {
	    /* Last arg to be pushed. */
	    done = 1;
	    argop = GenExpression(arg, Codes);
	    if (!gProject->itsOptions->useMC68881)
			SetLocSZ(argop, GetTPSize(GetTreeTP(arg)));
	    if (Param) {
			if (*pcount < Via(Param)->argcount) {
			    GenInst(M68op_MOVE, GetLocSZ(argop), argop, ParamLoc(Via(Param)->args[*pcount]), Codes);
			    (*pcount)++;
			} else
			    GenPascalPush(argop, Codes);
	    } else
			GenPascalPush(argop, Codes);
	    FreeTemp(argop);
	    result = GetTPSize(GetTreeTP(arg));
	    if (GetLocSZ(argop) == M68sz_byte) {
			/* bytes are really pushed as words. */
			result++;
	    }
	}
    }
    return result;
}

int
PushArguments(ParseTreeVia_t fcallnode, int ispascal, ParamRecVia_t Param, InstListVia_t Codes)
/*
 * Given a list of arguments to a function, push them onto the stack. The
 * argument expressions are evaluated before pushing them.  Push pascal
 * arguments backwards.
 */
{
    int                             result;
    int                             pcount;
    int                             argsleft = 0;
    int                             paramsleft = 0;
    ParseTreeVia_t                  arg;
    LocAMVia_t                      argop;
    LocAMVia_t                      temparg;
    int                             done;
    result = 0;
    done = 0;
    /*
     * For each argument, generate code for the expression, pushing the
     * result on the stack.
     */
    arg = Via(fcallnode)->b;
    if (ispascal) {
	pcount = 0;
	result = PushBackwards(arg, Param, &pcount, Codes);
    } else {
	if (Param) {
	    paramsleft = Via(Param)->argcount;
	    argsleft = CountArgs(arg);
	}
	while ((!done) && arg) {
	    if (Via(arg)->kind == PTF_argument_list) {
		argop = GenExpression(Via(arg)->a, Codes);
		if (!gProject->itsOptions->useMC68881)
		    SetLocSZ(argop, GetTPSize(GetTreeTP(Via(arg)->a)));
		if (GetLocSZ(argop) == M68sz_byte) {
		    temparg = TempDataReg(Codes);
		    GenInst(M68op_MOVE, GetLocSZ(argop), argop, temparg, Codes);
		    FreeTemp(argop);
		    GenInst(M68op_EXT, M68sz_word, temparg, NULL, Codes);
		    GenInst(M68op_EXT, M68sz_long, temparg, NULL, Codes);
		    SetLocSZ(temparg, M68sz_long);
		    argop = temparg;
		}
		if (GetLocSZ(argop) == M68sz_word) {
		    temparg = TempDataReg(Codes);
		    GenInst(M68op_MOVE, GetLocSZ(argop), argop, temparg, Codes);
		    FreeTemp(argop);
		    GenInst(M68op_EXT, M68sz_long, temparg, NULL, Codes);
		    SetLocSZ(temparg, M68sz_long);
		    argop = temparg;
		}
		if (paramsleft && (argsleft <= paramsleft)) {
		    if (argsleft < paramsleft)
			paramsleft = argsleft;
		    GenInst(M68op_MOVE, GetLocSZ(argop), argop, ParamLoc(Via(Param)->args[paramsleft - 1]), Codes);
		    paramsleft--;
		} else
		    GenPush(argop, Codes);
		argsleft--;
		FreeTemp(argop);
		result += GetTPSize(GetTreeTP(Via(arg)->a));
		if (GetTPSize(GetTreeTP(Via(arg)->a)) == 1) {
		    /* bytes are really pushed as longs. */
		    result += 3;
		}
		if (GetTPSize(GetTreeTP(Via(arg)->a)) == 2) {
		    /* shorts are really pushed as longs. */
		    result += 2;
		}
		arg = Via(arg)->b;
	    } else {
		/* Last arg to be pushed. */
		done = 1;
		argop = GenExpression(arg, Codes);
		if (!gProject->itsOptions->useMC68881)
		    SetLocSZ(argop, GetTPSize(GetTreeTP(arg)));
		if (GetLocSZ(argop) == M68sz_byte) {
		    temparg = TempDataReg(Codes);
		    GenInst(M68op_MOVE, GetLocSZ(argop), argop, temparg, Codes);
		    FreeTemp(argop);
		    GenInst(M68op_EXT, M68sz_word, temparg, NULL, Codes);
		    GenInst(M68op_EXT, M68sz_long, temparg, NULL, Codes);
		    SetLocSZ(temparg, M68sz_long);
		    argop = temparg;
		}
		if (GetLocSZ(argop) == M68sz_word) {
		    temparg = TempDataReg(Codes);
		    GenInst(M68op_MOVE, GetLocSZ(argop), argop, temparg, Codes);
		    FreeTemp(argop);
		    GenInst(M68op_EXT, M68sz_long, temparg, NULL, Codes);
		    SetLocSZ(temparg, M68sz_long);
		    argop = temparg;
		}
		if (paramsleft && (argsleft <= paramsleft)) {
		    if (argsleft < paramsleft)
			paramsleft = argsleft;
		    GenInst(M68op_MOVE, GetLocSZ(argop), argop, ParamLoc(Via(Param)->args[paramsleft - 1]), Codes);
		    paramsleft--;
		} else
		    GenPush(argop, Codes);
		argsleft--;
		FreeTemp(argop);
		result += GetTPSize(GetTreeTP(arg));
		if (GetTPSize(GetTreeTP(arg)) == 1) {
		    /* bytes are really pushed as longs. */
		    result += 3;
		}
		if (GetTPSize(GetTreeTP(arg)) == 2) {
		    /* short are really pushed as longs. */
		    result += 2;
		}
	    }
	}
    }
    return result;
}

void
GenCompareBranch2(LocAMVia_t leftop, ParseTreeVia_t right,
		  Opcode_t OP, LabSYMVia_t label,
		  InstListVia_t Codes)
{
	/* leftop must be a temporary.  If non float, it must be in a dreg */
    /*
     * Used for switch comparisons, so the control expression need be
     * evaluated only once.
     */

    LocAMVia_t                      rightop;
    if (isFloatingType(GetTreeTP(right))) {
	rightop = GenExpression(right, Codes);
	if (gProject->itsOptions->useMC68881) {
	    GenInst(M68op_FCMP, GetLocSZ(rightop), rightop, leftop, Codes);
	    switch (OP) {
	    case M68op_BEQ:
		OP = M68op_FBEQ;
		break;
	    case M68op_BNE:
		OP = M68op_FBNE;
		break;
	    case M68op_BLT:
		OP = M68op_FBLT;
		break;
	    case M68op_BGT:
		OP = M68op_FBGT;
		break;
	    case M68op_BLE:
		OP = M68op_FBLE;
		break;
	    case M68op_BGE:
		OP = M68op_FBGE;
		break;
	    }
	    GenInst(OP, M68sz_none, BuildLabelLoc(label), NULL, Codes);
	} else {
	    rightop = DictateTempFloat(rightop, Codes);
	    GenSANECall(FFEXT + FOCMP, rightop, leftop, Codes);
	    switch (OP) {
	    case M68op_BEQ:
		OP = M68op_BEQ;
		break;
	    case M68op_BNE:
		OP = M68op_BNE;
		break;
	    case M68op_BLT:
		OP = M68op_BCS;
		break;
	    case M68op_BGT:
		OP = M68op_BGT;
		break;
	    case M68op_BLE:
		OP = M68op_BLS;
		break;
	    case M68op_BGE:
		OP = M68op_BGE;
		break;
	    }
	    GenInst(OP, M68sz_none, BuildLabelLoc(label), NULL, Codes);
	}
	FreeTemp(rightop);
	FreeTemp(leftop);
    } else {
	rightop = GenExpression(right, Codes);
	GenInst(M68op_CMP, GetLocSZ(leftop), rightop, leftop, Codes);
	GenInst(OP, M68sz_none, BuildLabelLoc(label), NULL, Codes);
	FreeTemp(rightop);
    }
}

void
GenCompareBranch(ParseTreeVia_t left, ParseTreeVia_t right,
		 Opcode_t OP, LabSYMVia_t label,
		 InstListVia_t Codes)
{
    /*
     * Compares the two expressions, generating a branch (contained in the OP
     * arg) afterwards.
     */
    LocAMVia_t                      leftop;
    LocAMVia_t                      rightop;
    if (isFloatingType(GetTreeTP(left))) {
	leftop = GenExpression(left, Codes);
	leftop = DictateTempFloat(leftop, Codes);
	rightop = GenExpression(right, Codes);
	if (gProject->itsOptions->useMC68881) {
	    GenInst(M68op_FCMP, GetLocSZ(rightop), rightop, leftop, Codes);
	    switch (OP) {
	    case M68op_BEQ:
		OP = M68op_FBEQ;
		break;
	    case M68op_BNE:
		OP = M68op_FBNE;
		break;
	    case M68op_BLT:
		OP = M68op_FBLT;
		break;
	    case M68op_BGT:
		OP = M68op_FBGT;
		break;
	    case M68op_BLE:
		OP = M68op_FBLE;
		break;
	    case M68op_BGE:
		OP = M68op_FBGE;
		break;
	    }
	    GenInst(OP, M68sz_none, BuildLabelLoc(label), NULL, Codes);
	} else {
	    rightop = DictateTempFloat(rightop, Codes);
	    GenSANECall(FFEXT + FOCMP, rightop, leftop, Codes);
	    switch (OP) {
	    case M68op_BEQ:
		OP = M68op_BEQ;
		break;
	    case M68op_BNE:
		OP = M68op_BNE;
		break;
	    case M68op_BLT:
		OP = M68op_BCS;
		break;
	    case M68op_BGT:
		OP = M68op_BGT;
		break;
	    case M68op_BLE:
		OP = M68op_BLS;
		break;
	    case M68op_BGE:
		OP = M68op_BGE;
		break;
	    }
	    GenInst(OP, M68sz_none, BuildLabelLoc(label), NULL, Codes);
	}
	FreeTemp(rightop);
	FreeTemp(leftop);
    } else {
	if (isUnsignedType(GetTreeTP(left))) {
	    leftop = GenExpression(left, Codes);
	    rightop = GenExpression(right, Codes);
	    leftop = DictateAnyDReg(leftop, Codes);
	    GenInst(M68op_CMP, GetLocSZ(leftop), rightop, leftop, Codes);
	    switch (OP) {
	    case M68op_BLT:
		OP = M68op_BCS;
		break;
	    case M68op_BGT:
		OP = M68op_BHI;
		break;
	    case M68op_BLE:
		OP = M68op_BLS;
		break;
	    case M68op_BGE:
		OP = M68op_BCC;
		break;
	    }
	    GenInst(OP, M68sz_none, BuildLabelLoc(label), NULL, Codes);
	    FreeTemp(rightop);
	    FreeTemp(leftop);
	} else {
	    leftop = GenExpression(left, Codes);
	    rightop = GenExpression(right, Codes);
	    /*
	     * TODO Here would be a good usage of the extended reg allocator
	     * which would allow specification of a noconflict loc
	     */
	    leftop = DictateAnyDReg(leftop, Codes);
	    GenDeConflictize(&leftop, &rightop, Codes);
	    GenInst(M68op_CMP, GetLocSZ(leftop), rightop, leftop, Codes);
	    GenInst(OP, M68sz_none, BuildLabelLoc(label), NULL, Codes);
	    FreeTemp(rightop);
	    FreeTemp(leftop);
	}
    }
}

void
GenCompareZero(ParseTreeVia_t expr, LabSYMVia_t label,
	       InstListVia_t Codes)
{
    /*
     * Compares the given expression with 0, and branches to the given label
     * if the expression is zero.
     */

    ParseTreeVia_t                  zeroexpr;
    FoldValue_t                     testK;
    if (!expr) {
		GenInst(M68op_BRA, M68sz_none, BuildLabelLoc(label), NULL, Codes);
		return;
    }
    ConstExprValue(expr, &testK);
    if (testK.isK) {
	if (testK.isint) {
	    if (testK.intval) {
		return;
	    } else {
		GenInst(M68op_BRA, M68sz_none, BuildLabelLoc(label), NULL, Codes);
		return;
	    }
	}
    }
    /*
     * First we build a parse tree node equal to constant zero, so we can
     * compare the argument with it.
     */
    zeroexpr = BuildTreeNode(PTF_intconstant, NULL, NULL, NULL);
    SetTreeTP(zeroexpr, GetTreeTP(expr));
    Via(zeroexpr)->data.number = 0;
    if (isFloatingType(GetTreeTP(expr))) {
		SetTreeTP(zeroexpr, BuildTypeRecord(0, TRC_int, SGN_signed));
		zeroexpr = TypeConvert(zeroexpr, BuildTypeRecord(0, TRC_longdouble,
								 SGN_unknown));
    }
    /*
     * In certain cases here, we can simply reverse the direction of the
     * branch which would normally be generated by the boolean expression in
     * expr, and branch instead to the label passed here, instead of
     * generating the intermediate boolean value of 0 or 1.
     */
    if (isIntegralType(GetTreeTP(expr)) && !isUnsignedType(GetTreeTP(expr))) {
	switch (Via(expr)->kind) {
	case PTF_lessthan:
	    GenCompareBranch(Via(expr)->a, Via(expr)->b, M68op_BGE, label, Codes);
	    break;
	case PTF_greaterthan:
	    GenCompareBranch(Via(expr)->a, Via(expr)->b, M68op_BLE, label, Codes);
	    break;
	case PTF_lessthaneq:
	    GenCompareBranch(Via(expr)->a, Via(expr)->b, M68op_BGT, label, Codes);
	    break;
	case PTF_greaterthaneq:
	    GenCompareBranch(Via(expr)->a, Via(expr)->b, M68op_BLT, label, Codes);
	    break;
	case PTF_equal:
	    GenCompareBranch(Via(expr)->a, Via(expr)->b, M68op_BNE, label, Codes);
	    break;
	case PTF_notequal:
	    GenCompareBranch(Via(expr)->a, Via(expr)->b, M68op_BEQ, label, Codes);
	    break;
	default:
	    GenCompareBranch(expr, zeroexpr, M68op_BEQ, label, Codes);
	    break;
	}
    } else {
		GenCompareBranch(expr, zeroexpr, M68op_BEQ, label, Codes);
    }
    FreeTree(zeroexpr);
}

void
GenCompareNonZero(ParseTreeVia_t expr,
		  LabSYMVia_t label,
		  InstListVia_t Codes)
{
    /*
     * Compares the given expression with 0, and branches to the given label
     * if the expression is non-zero.
     */

    ParseTreeVia_t                  zeroexpr;
    FoldValue_t                     testK;
    if (!expr)
	return;
    ConstExprValue(expr, &testK);
    if (testK.isK) {
	if (testK.isint) {
	    if (testK.intval) {
		GenInst(M68op_BRA, M68sz_none, BuildLabelLoc(label), NULL, Codes);
		return;
	    } else {
		return;
	    }
	}
    }
    zeroexpr = BuildTreeNode(PTF_intconstant, NULL, NULL, NULL);
    SetTreeTP(zeroexpr, GetTreeTP(expr));
    Via(zeroexpr)->data.number = 0;
    if (isFloatingType(GetTreeTP(expr))) {
	SetTreeTP(zeroexpr, BuildTypeRecord(0, TRC_int, SGN_signed));
	zeroexpr = TypeConvert(zeroexpr, BuildTypeRecord(0, TRC_longdouble,
							 SGN_unknown));
    }
    /*
     * In certain cases here, we can simply reverse the direction of the
     * branch which would normally be generated by the boolean expression in
     * expr, and branch instead to the label passed here, instead of
     * generating the intermediate boolean value of 0 or 1.
     */
    if (isIntegralType(GetTreeTP(expr)) && !isUnsignedType(GetTreeTP(expr))) {
	switch (Via(expr)->kind) {
	case PTF_lessthan:
	    GenCompareBranch(Via(expr)->a, Via(expr)->b, M68op_BLT, label, Codes);
	    break;
	case PTF_greaterthan:
	    GenCompareBranch(Via(expr)->a, Via(expr)->b, M68op_BGT, label, Codes);
	    break;
	case PTF_lessthaneq:
	    GenCompareBranch(Via(expr)->a, Via(expr)->b, M68op_BLE, label, Codes);
	    break;
	case PTF_greaterthaneq:
	    GenCompareBranch(Via(expr)->a, Via(expr)->b, M68op_BGE, label, Codes);
	    break;
	case PTF_equal:
	    GenCompareBranch(Via(expr)->a, Via(expr)->b, M68op_BEQ, label, Codes);
	    break;
	case PTF_notequal:
	    GenCompareBranch(Via(expr)->a, Via(expr)->b, M68op_BNE, label, Codes);
	    break;
	default:
	    GenCompareBranch(expr, zeroexpr, M68op_BNE, label, Codes);
	    break;
	}
    } else {
	GenCompareBranch(expr, zeroexpr, M68op_BNE, label, Codes);
    }
    FreeTree(zeroexpr);
}

LocAMVia_t
GenLibraryCall(LocAMVia_t leftop, LocAMVia_t rightop,
	       LabSYMVia_t label, InstListVia_t Codes)
{
    /*
     * Generate a call to the long math library. Arguments are the two
     * operands and the label of the library routine being called.
     */
    LocAMVia_t                      result;
    SaveTheRegisters(Codes);	// Changed from SaveDataRegisters() 24 Mar 1992
    /*
     * The code below is designed to ensure that nothing gets validated in
     * the process of filling d0 and 1.  The size of the move is always long
     * because a reg is always spilled as long and the destination is always
     * long.  QQQQ Should we ensure that the destination dreg is zeroed just
     * in case of a byte or word operand, when not dealing with spilled ops ?
     */
    if (!((GetLocAM(leftop) == M68am_DReg) && (GetLocDReg(leftop) == 0))) {
	if ((GetLocAM(leftop) == M68am_DReg) && (GetLocStatus(leftop) == M68os_spilled)) {
	    Genmove(M68sz_long, Via(GetLocSlot(leftop))->SlotLoc, BuildDRegLocation(0), Codes);
	} else {
	    Genmove(GetLocSZ(leftop), leftop, BuildDRegLocation(0), Codes);
	}
    }
    if (!((GetLocAM(rightop) == M68am_DReg) && (GetLocDReg(rightop) == 1))) {
	if ((GetLocAM(rightop) == M68am_DReg) && (GetLocStatus(rightop) == M68os_spilled)) {
	    Genmove(M68sz_long, Via(GetLocSlot(rightop))->SlotLoc, BuildDRegLocation(1), Codes);
	} else {
	    Genmove(GetLocSZ(rightop), rightop, BuildDRegLocation(1), Codes);
	}
    }
	GenInst(M68op_JSR, M68sz_none,
	      BuildARegLabelDisplace(5, label), NULL, Codes);
    /*
     * We allocate a spill slot to return the result.  The whole library call
     * should not disturb the register allocation situation at all.
     */
#define IWISH
#ifdef IWISH
    result = TempDataReg(Codes);
    if (GetLocDReg(result) != 0) {
	Genmove(M68sz_long, BuildDRegLocation(0), result, Codes);
    }
    SetLocSZ(result, M68sz_long);
#else
    {
	SpillSlotVia_t                  slot;
	slot = GetSpillSlot(4);
	result = Via(slot)->SlotLoc;
	Genmove(M68sz_long, BuildDRegLocation(0), result, Codes);
	SetLocSZ(result, M68sz_long);
    }
#endif
    FreeTemp(leftop);
    FreeTemp(rightop);
    return result;
}

LocAMVia_t
GenMulUnsigned(Opcode_t OP, LocAMVia_t leftop, LocAMVia_t rightop,
	       InstListVia_t Codes)
{
    /*
     * General routine to generate an unsigned multiply of two integers.
     * Depending on the current compilation options, generated code may vary.
     */
    if (GetLocSZ(leftop) > 2) {
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
	} else {
	    leftop = DictateTempDReg(leftop, Codes);
	    GenDeConflictize(&leftop, &rightop, Codes);
	    switch (OP) {
	    case M68op_MULS:
		GenInst(M68op_MULU, M68sz_long, rightop, leftop, Codes);
		break;
	    case M68op_DIVS:
		GenInst(M68op_DIVU, M68sz_long, rightop, leftop, Codes);
		break;
	    }
	}
    } else {
	leftop = DictateTempDReg(leftop, Codes);
	rightop = DictateTempDReg(rightop, Codes);
	GenDeConflictize(&leftop, &rightop, Codes);
	switch (OP) {
	case M68op_MULS:
	    GenInst(M68op_MULU, M68sz_none, rightop, leftop, Codes);
	    break;
	case M68op_DIVS:
	    GenInst(M68op_DIVU, M68sz_none, rightop, leftop, Codes);
	    break;
	}
    }
    return leftop;
}

LocAMVia_t
GenMulSigned(Opcode_t OP, LocAMVia_t leftop, LocAMVia_t rightop,
	     InstListVia_t Codes)
/*
 * General routine to generate an signed multiply of two integers. Depending
 * on the current compilation options, generated code may vary.
 */
{
    if (GetLocSZ(leftop) > 2) {
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
	} else {
	    leftop = DictateTempDReg(leftop, Codes);
	    GenInst(OP, M68sz_long, rightop, leftop, Codes);
	}
    } else {
	leftop = DictateTempDReg(leftop, Codes);
	rightop = DictateTempDReg(rightop, Codes);
	GenDeConflictize(&leftop, &rightop, Codes);
	GenInst(OP, M68sz_none, rightop, leftop, Codes);
    }
    return leftop;
}

int
isDRegMode(LocAMVia_t loc)
{
    if (loc) {
	switch (GetLocAM(loc)) {
	    case M68am_DReg:
	    return 1;
	    break;
	default:
	    return 0;
	    break;
	}
    } else {
	return 0;
    }
}

int
isARegMode(LocAMVia_t loc)
{
    if (loc) {
	switch (GetLocAM(loc)) {
	    case M68am_ARegDirect:
	    case M68am_ARegIndirect:
	    case M68am_ARegPostInc:
	    case M68am_ARegPreDec:
	    case M68am_ARegLabelDisplace:
	    case M68am_ARegDisplace:
	    case M68am_ARegDisplaceFIELD:
	    case M68am_ARegDispIndx:
	    return 1;
	    break;
	default:
	    return 0;
	    break;
	}
    } else {
	return 0;
    }
}

LocAMVia_t
GenTypeChange(ParseTreeVia_t expr, InstListVia_t Codes)
/* This routine handles conversions between various types. */
{
    LocAMVia_t                      result = NULL;
    LocAMVia_t                      temploc;
    if (isFloatingType(GetTreeTP(expr))) {
	if (isIntegralType(GetTreeTP(Via(expr)->a))) {
	    if (gProject->itsOptions->useMC68881) {
		temploc = GenExpression(Via(expr)->a, Codes);
		/*
		 * We will assume that any expression which reaches here has
		 * already been cast to long.
		 */
		result = TempFloatReg(Codes);
		GenInst(M68op_FMOVE, GetLocSZ(temploc), temploc, result, Codes);
	    } else {
		/*
		 * We will assume that any expression which reaches here has
		 * already been cast to long.
		 */
		LocAMVia_t                      theint;
		LocAMVia_t                      ftemp;
		SpillSlotVia_t                  slot;
		ftemp = TempFloatReg(Codes);
		temploc = GenExpression(Via(expr)->a, Codes);
		slot = GetSpillSlot(M68sz_long);
		theint = Via(slot)->SlotLoc;
		SetLocSZ(theint, M68sz_long);
		GenInst(M68op_MOVE, M68sz_long, temploc, theint, Codes);
		FreeTemp(temploc);
		GenSANECall(FOZ2X + FFLNG, theint, ftemp, Codes);
		FreeTemp(theint);
		result = ftemp;
		/*
		 * This routine ALWAYS converts its operand to SANE extended
		 */
	    }
	} else if (isFloatingType(GetTreeTP(Via(expr)->a))) {
#ifdef Undefined
	    /* These will be different for Mac and SANE */
	    if (isExtendedFloatingType(GetTreeTP(Via(expr)->a))) {
		result = GenExpression(Via(expr)->a, Codes);
	    } else {
		if (!gProject->itsOptions->useMC68881) {
		    LocAMVia_t                      ftemp;
		    ftemp = TempFloatReg(Codes);
		    result = GenExpression(Via(expr)->a, Codes);
		    Genmove(GetLocSZ(result), result, ftemp, Codes);
		    FreeTemp(result);
		    result = ftemp;
		} else {
		    if (GetTPKind(GetTreeTP(expr)) == TRC_float) {
			if (GetTPKind(GetTreeTP(Via(expr)->a)) != TRC_float) {
			    /* Convert double to float. */
			    LocAMVia_t                      temploc2;
			    SpillSlotVia_t                  slot;
			    temploc = GenExpression(Via(expr)->a, Codes);
			    temploc2 = TempFloatReg(Codes);
			    GenInst(M68op_FMOVE, GetLocSZ(temploc), temploc, temploc2, Codes);
			    Via(temploc2)->SZ = M68sz_single;
			    slot = GetSpillSlot(SizeOfFloat);
			    result = Via(slot)->SlotLoc;
			    SetLocSZ(result, M68sz_single);
			    GenInst(M68op_FMOVE, M68sz_single, temploc2, result, Codes);
			    FreeTemp(temploc2);
			} else {
			    result = GenExpression(Via(expr)->a, Codes);
			    SetLocSZ(result, M68sz_single);
			}
		    } else if (GetTPKind(GetTreeTP(Via(expr)->a)) == TRC_float) {
			/* Convert float to double. */
			if (GetTPKind(GetTreeTP(expr)) != TRC_float) {
			    LocAMVia_t                      temploc2;
			    SpillSlotVia_t                  slot;
			    temploc = GenExpression(Via(expr)->a, Codes);
			    temploc2 = TempFloatReg(Codes);
			    GenInst(M68op_FMOVE, GetLocSZ(temploc), temploc, temploc2, Codes);
			    Via(temploc2)->SZ = M68sz_double;
			    slot = GetSpillSlot(SizeOfDouble);
			    result = Via(slot)->SlotLoc;
			    SetLocSZ(result, M68sz_double);
			    GenInst(M68op_FMOVE, M68sz_double, temploc2, result, Codes);
			    FreeTemp(temploc2);
			} else {
			    result = GenExpression(Via(expr)->a, Codes);
			    SetLocSZ(result, M68sz_single);
			}
		    } else {
			result = GenExpression(Via(expr)->a, Codes);
		    }
		}
	    }
#else
	    result = GenExpression(Via(expr)->a, Codes);
	    return result;
#endif
	} else {
	    Gen68Error("Illegal conversion");
	}
	if (!gProject->itsOptions->useMC68881)
	    SetLocSZ(result, GetTPSize(GetTreeTP(expr)));
    } else if (isIntegralType(GetTreeTP(expr))) {
	if (isIntegralType(GetTreeTP(Via(expr)->a))) {
	    /* Most conversions are here. */
	    if (isBitFieldType(GetTreeTP(expr))) {
		/*
		 * Convert integer to bit field - what is this useful for ?
		 */
		result = GenExpression(Via(expr)->a, Codes);
		return result;
	    } else if (isBitFieldType(GetTreeTP(Via(expr)->a))) {
		/* Convert bit field to integer, this is the typical case. */
		unsigned long                   bitmask;
		temploc = GenExpression(Via(expr)->a, Codes);
		result = TempDataReg(Codes);
		if (GetLocSZ(temploc) != M68sz_long)
		    GenInst(M68op_MOVEQ, M68sz_none, BuildImmediate(0, M68sz_long), result, Codes);
		GenInst(M68op_MOVE, GetLocSZ(temploc), temploc, result, Codes);
		FreeTemp(temploc);
		/* Construct a mask, and AND immed */
		bitmask = MakeMask(temploc);
		GenInst(M68op_AND, GetLocSZ(temploc), BuildImmediate(bitmask, M68sz_long), result, Codes);
		ShiftRight(result, GetLocFieldBits(temploc).StartBit, Codes);
		SetLocSZ(result, GetTPSize(GetTreeTP(expr)));
		if (isSignedType(GetTreeTP(Via(expr)->a))) {
		    /*
		     * Here, we test the highest bit of the bit field, and if
		     * it is true, then we OR the result with a mask setting
		     * all the most significant bits.
		     */
		    unsigned long                   himask;
		    LabSYMVia_t                     over;
		    over = MakeEccLabel(NextEccLabel++);
		    GenInst(M68op_BTST, M68sz_none,
			    BuildImmediate(GetLocFieldBits(temploc).EndBit - GetLocFieldBits(temploc).StartBit, M68sz_long), result, Codes);
		    GenInst(M68op_BEQ, M68sz_none, BuildLabelLoc(over), NULL, Codes);
		    himask =
			MakeTheMask(GetLocFieldBits(temploc).EndBit - GetLocFieldBits(temploc).StartBit
				    + 1, 32);
		    GenInst(M68op_OR, GetLocSZ(temploc), BuildImmediate(himask, M68sz_long), result, Codes);
		    GenLabel(over, Codes);
		}
		return result;
	    }
	    if (isUnsignedType(GetTreeTP(expr))) {
		if (isUnsignedType(GetTreeTP(Via(expr)->a))) {
		    if (GetTPSize(GetTreeTP(expr)) > GetTPSize(GetTreeTP(Via(expr)->a))) {
			temploc = GenExpression(Via(expr)->a, Codes);
			result = TempDataReg(Codes);
			if (GetLocSZ(temploc) != M68sz_long)
			    GenInst(M68op_MOVEQ, M68sz_none, BuildImmediate(0, M68sz_long), result, Codes);
			GenInst(M68op_MOVE, GetLocSZ(temploc), temploc, result,
				Codes);
			FreeTemp(temploc);
			SetLocSZ(result, GetTPSize(GetTreeTP(expr)));
		    } else if (GetTPSize(GetTreeTP(expr)) < GetTPSize(GetTreeTP(Via(expr)->a))) {
			temploc = GenExpression(Via(expr)->a, Codes);
			result = DictateTempDReg(temploc, Codes);
			SetLocSZ(result, GetTPSize(GetTreeTP(expr)));
		    } else {
			result = GenExpression(Via(expr)->a, Codes);
			SetLocSZ(result, GetTPSize(GetTreeTP(expr)));
		    }
		} else {
		    if (GetTPSize(GetTreeTP(expr)) > GetTPSize(GetTreeTP(Via(expr)->a))) {
			temploc = GenExpression(Via(expr)->a, Codes);
			result = TempDataReg(Codes);
			if (GetLocSZ(temploc) != M68sz_long)
			    GenInst(M68op_MOVEQ, M68sz_none, BuildImmediate(0, M68sz_long), result, Codes);
			GenInst(M68op_MOVE, GetLocSZ(temploc), temploc, result,
				Codes);
			FreeTemp(temploc);
			SetLocSZ(result, GetTPSize(GetTreeTP(expr)));
		    } else if (GetTPSize(GetTreeTP(expr)) < GetTPSize(GetTreeTP(Via(expr)->a))) {
			temploc = GenExpression(Via(expr)->a, Codes);
			result = DictateTempDReg(temploc, Codes);
			SetLocSZ(result, GetTPSize(GetTreeTP(expr)));
		    } else {
			result = GenExpression(Via(expr)->a, Codes);
			SetLocSZ(result, GetTPSize(GetTreeTP(expr)));
		    }
		}
	    } else {
		if (isUnsignedType(GetTreeTP(Via(expr)->a))) {
		    if (GetTPSize(GetTreeTP(expr)) > GetTPSize(GetTreeTP(Via(expr)->a))) {
			temploc = GenExpression(Via(expr)->a, Codes);
			result = TempDataReg(Codes);
			if (GetLocSZ(temploc) != M68sz_long)
			    GenInst(M68op_MOVEQ, M68sz_none, BuildImmediate(0, M68sz_long), result, Codes);
			GenInst(M68op_MOVE, GetLocSZ(temploc), temploc, result,
				Codes);
			FreeTemp(temploc);
			SetLocSZ(result, GetTPSize(GetTreeTP(expr)));
		    } else if (GetTPSize(GetTreeTP(expr)) < GetTPSize(GetTreeTP(Via(expr)->a))) {
			temploc = GenExpression(Via(expr)->a, Codes);
			result = DictateTempDReg(temploc, Codes);
			SetLocSZ(result, GetTPSize(GetTreeTP(expr)));
		    } else {
			result = GenExpression(Via(expr)->a, Codes);
			SetLocSZ(result, GetTPSize(GetTreeTP(expr)));
		    }
		} else {
		    if (GetTPSize(GetTreeTP(expr)) > GetTPSize(GetTreeTP(Via(expr)->a))) {
			temploc = GenExpression(Via(expr)->a, Codes);
			result = TempDataReg(Codes);
			GenInst(M68op_MOVE, GetLocSZ(temploc), temploc, result, Codes);
			FreeTemp(temploc);
			if (GetTPSize(GetTreeTP(Via(expr)->a)) == 1) {
			    GenInst(M68op_EXT, M68sz_word, result, NULL, Codes);
			}
			if (GetTPSize(GetTreeTP(expr)) == 4) {
			    GenInst(M68op_EXT, M68sz_long, result, NULL, Codes);
			}
			SetLocSZ(result, GetTPSize(GetTreeTP(expr)));
		    } else if (GetTPSize(GetTreeTP(expr)) < GetTPSize(GetTreeTP(Via(expr)->a))) {
			temploc = GenExpression(Via(expr)->a, Codes);
			if (isConstantLoc(temploc)) {
				result = temploc;
			}
			else {
				result = DictateTempDReg(temploc, Codes);
			}
			SetLocSZ(result, GetTPSize(GetTreeTP(expr)));
		    } else {
			result = GenExpression(Via(expr)->a, Codes);
			SetLocSZ(result, GetTPSize(GetTreeTP(expr)));
		    }
		}
	    }
	} else if (isFloatingType(GetTreeTP(Via(expr)->a))) {
	    if (gProject->itsOptions->useMC68881) {
		/* We will assume that the destination type is long. */
		temploc = GenExpression(Via(expr)->a, Codes);
		result = TempFloatReg(Codes);
		GenInst(M68op_FINTRZ, GetLocSZ(temploc), temploc, result, Codes);
		SetLocSZ(result, M68sz_long);
		FreeTemp(temploc);
	    } else {
		/* We will assume that the destination type is long. */
		SpillSlotVia_t                  slot;
		temploc = GenExpression(Via(expr)->a, Codes);
		temploc = DictateTempFloat(temploc, Codes);
		slot = GetSpillSlot(M68sz_long);
		result = Via(slot)->SlotLoc;
		GenSANECall(FFEXT + FOTTI, temploc, NULL, Codes);
		GenPushEA(temploc, Codes);
		SaveDataRegisters(Codes);
		    GenInst(M68op_JSR, M68sz_none,
			    BuildARegLabelDisplace(5, LibLabelXTOI), NULL, Codes);
		GenInst(M68op_MOVE, M68sz_long, BuildDRegLocation(0), result, Codes);
		SetLocSZ(result, M68sz_long);
		FreeTemp(temploc);
	    }
	} else if (isPointerType(GetTreeTP(Via(expr)->a))) {
	    temploc = GenExpression(Via(expr)->a, Codes);
	    result = TempDataReg(Codes);
	    GenInst(M68op_MOVE, M68sz_long, temploc, result, Codes);
	    SetLocSZ(result, M68sz_long);
	    FreeTemp(temploc);
	} else {
	    Gen68Error("Illegal conversion");
	}
    } else if (isPointerType(GetTreeTP(expr))) {
	if (isPointerType(GetTreeTP(Via(expr)->a))) {
	    result = GenExpression(Via(expr)->a, Codes);
	} else if (isIntegralType(GetTreeTP(Via(expr)->a))) {
	    result = GenExpression(Via(expr)->a, Codes);
	    result = DictateTempAReg(result, Codes);
	    SetLocSZ(result, M68sz_long);

	} else if (isArrayType(GetTreeTP(Via(expr)->a)) ||
		   isStructUnionType(GetTreeTP(Via(expr)->a))) {
	    result = GenAddrOf(Via(expr)->a, Codes);
	} else {
	    Gen68Error("Illegal conversion");
	}
    } else if (isStructUnionType(GetTreeTP(expr))) {
	if (isStructUnionType(GetTreeTP(Via(expr)->a))) {
	    result = GenExpression(Via(expr)->a, Codes);
	} else {
	    Gen68Error("Illegal conversion");
	}
    } else if (isVoidType(GetTreeTP(expr))) {
	result = GenExpression(Via(expr)->a, Codes);
	result = NULL;
    } else {
	result = GenExpression(Via(expr)->a, Codes);
	SetLocSZ(result, M68_TypeSize(GetTreeTP(expr)));
    }
    return result;
}
