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
 * This file contains routines for 68k register allocation.
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


#pragma segment RegAlloc


SpillSlotVia_t
RawSpillSlot(void)
{
    register SpillSlotVia_t         raw;
    raw = Ealloc(sizeof(SpillSlot_t));
    Via(raw)->next = NULL;
    Via(raw)->local = 0;
    Via(raw)->status = 0;
    Via(raw)->SlotLoc = NULL;
    Via(raw)->SlotSize = 0;
    return raw;
}

void
FreePerms(void)
{
    /* Set all perm registers to available. */
    int                             ndx;

    ndx = MAXTEMPDATAREG + 1;
    while (ndx <= 7) {
	TempDatas[ndx].status = M68rs_Available;
	TempDatas[ndx].holder = NULL;
	TempDatas[ndx].waiting = 0;
	TempDatas[ndx].count = 0;
	ndx++;
    }
    ndx = MAXTEMPADDRREG + 1;
    while (ndx <= 7) {
	TempAddrs[ndx].status = M68rs_Available;
	TempAddrs[ndx].holder = NULL;
	TempAddrs[ndx].waiting = 0;
	TempAddrs[ndx].count = 0;
	ndx++;
    }
    ndx = MAXTEMPFLOATREG + 1;
    while (ndx <= 6) {
	TempAddrs[ndx].status = M68rs_Available;
	TempAddrs[ndx].holder = NULL;
	TempAddrs[ndx].waiting = 0;
	TempAddrs[ndx].count = 0;
	ndx++;
    }
}

void
FreeAllRegs(void)
{
    /* Set all temp registers to available. */
    int                             ndx;

    ndx = 0;
    while (ndx <= MAXTEMPDATAREG) {
	/* TODO Check here to see if it is hanging */
	TempDatas[ndx].status = M68rs_Available;
	TempDatas[ndx].holder = NULL;
	TempDatas[ndx].waiting = 0;
	ndx++;
    }
    ndx = 0;
    while (ndx <= MAXTEMPADDRREG) {
	/* TODO Check here to see if it is hanging */
	TempAddrs[ndx].status = M68rs_Available;
	TempAddrs[ndx].holder = NULL;
	TempAddrs[ndx].waiting = 0;
	ndx++;
    }
    ndx = 0;
    while (ndx <= MAXTEMPFLOATREG) {
	TempFloats[ndx].status = M68rs_Available;
	TempFloats[ndx].holder = NULL;
	TempFloats[ndx].waiting = 0;
	ndx++;
    }
    NextTempFloatReg = 0;
    NextTempDataReg = 0;
    NextTempAddrReg = 0;
}

SpillSlotVia_t
GetSpillSlot(int size)
/*
 * A spill slot is a temporary unit of storage, allocated on the stack frame
 * as if it were a local variable.  When temp registers spill, they are moved
 * to a spill slot.
 */
{
    SpillSlotVia_t                  result;
    SpillSlotVia_t                  cur;
    int                             realsize;
    int                             offset;
    int                             truesize;
    /*
     * Search the spill list for a slot of the given size which is free. If
     * not found, create a new one by expanding the stack frame (in the
     * global variable LinkInst) by size, and returning the slot.
     */
    truesize = size;
    switch (size) {
    case M68sz_single:
	size = SizeOfFloat;
	break;
    case M68sz_double:
	size = SizeOfDouble;
	break;
    case M68sz_extended:
	size = SizeOfLongDouble;
	break;
    default:
	break;
    }
    realsize = size;
    if (size % 2) {
	size++;
    }
    cur = SpillList;
    while (cur) {
	if (Via(cur)->SlotSize == size) {
	    if (Via(cur)->status == M68ss_free) {
		Via(cur)->status = M68ss_used;
		return cur;
	    }
	}
	cur = Via(cur)->next;
    }
    result = RawSpillSlot();

    Via(result)->SlotSize = size;
    Via(result)->next = SpillList;
    SpillList = result;
    Via(result)->status = M68ss_used;
#ifdef Undefined
    offset = GetLocConstant(Via(LinkInst)->right) - realsize;
#else
    offset = GetLocConstant(Via(LinkInst)->right) - size;	/* 27 Mar 1992 */
#endif
    SetLocConstant(Via(LinkInst)->right, GetLocConstant(Via(LinkInst)->right) - size);
    Via(result)->SlotLoc = BuildAutoLoc(offset, truesize);
    SetLocSlot(Via(result)->SlotLoc, result);

    return result;
}

void
KillSpillList(void)
{
    SpillSlotVia_t                  cur;
    SpillSlotVia_t                  nxt;
    cur = SpillList;
    while (cur) {
	nxt = Via(cur)->next;
	Efree(cur);
	cur = nxt;
    }
}

SpillSlotVia_t
SpillTemp(LocAMVia_t pushee, InstListVia_t Codes)
/* Spills the value in a temporary register... */
{
    SpillSlotVia_t                  result;
    /*
     * A SANE FP temp will never be spilled, because it is itself a spill
     * slot.
     */
    if (!isTempReg(pushee)) {
	Gen68Error("Spill temp called on non temp");
    }
    if (GetLocAM(pushee) == M68am_FReg) {
	if (!gProject->itsOptions->useMC68881) {
	    Gen68Error("FReg access with 68881 switch off !");
	}
	result = GetSpillSlot(8);
	SetLocSZ(Via(result)->SlotLoc, M68sz_double);
	SetLocSlot(pushee, result);
	Genmove(M68sz_double, pushee, Via(result)->SlotLoc, Codes);
	SetLocStatus(pushee, M68os_spilled);
	return result;
    } else {
	if (isARegMode(pushee)) {
	    LocAMVia_t                      reg;
	    result = GetSpillSlot(4);
	    SetLocSlot(pushee, result);
	    reg = BuildARegDirect(GetLocAReg(pushee));
	    Genmove(M68sz_long, reg, Via(result)->SlotLoc, Codes);
	    SetLocStatus(pushee, M68os_spilled);
	    return result;
	} else {
	    result = GetSpillSlot(4);
	    SetLocSlot(pushee, result);
	    Genmove(M68sz_long, pushee, Via(result)->SlotLoc, Codes);
	    SetLocStatus(pushee, M68os_spilled);
	    return result;
	}
    }
}

void
UnSpillTemp(LocAMVia_t freed, InstListVia_t Codes)
/* unspills a temp register. */
{
    if (isARegMode(freed)) {
	LocAMVia_t                      regd;
	regd = BuildARegDirect(GetLocAReg(freed));
	Via(GetLocSlot(freed))->status = M68ss_free;
	SetLocStatus(freed, M68os_valid);
	Genmove(M68sz_long, Via(GetLocSlot(freed))->SlotLoc, regd, Codes);
	SetLocSlot(freed, NULL);
    } else {
	Via(GetLocSlot(freed))->status = M68ss_free;
	SetLocStatus(freed, M68os_valid);
	Genmove(M68sz_long, Via(GetLocSlot(freed))->SlotLoc, freed, Codes);
	SetLocSlot(freed, NULL);
    }
}

int
isPermReg(LocAMVia_t loc)
{
    int                             result;
    result = 0;
    if (loc) {
	switch (GetLocAM(loc)) {
	case M68am_DReg:
	    if (GetLocDReg(loc) > MAXTEMPDATAREG) {
		result = 1;
	    }
	    break;
	case M68am_ARegDirect:
	case M68am_ARegIndirect:
	case M68am_ARegPostInc:
	case M68am_ARegPreDec:
	case M68am_ARegDisplace:
	case M68am_ARegDisplaceFIELD:
	case M68am_ARegDispIndx:
	    if (GetLocAReg(loc) > MAXTEMPADDRREG) {
		if ((GetLocAReg(loc) != 7) && (GetLocAReg(loc) != FRAME) && (GetLocAReg(loc)
						       != 5))
		    result = 1;
	    }
	    break;
	default:
	    break;
	}
    }
    return result;
}

int
isTempReg(LocAMVia_t loc)
/*
 * Returns true if the location passed is a temporary addr or data or 68881
 * float register.
 */
{
    int                             result;
    result = 0;
    if (loc) {
	switch (GetLocAM(loc)) {
	case M68am_FSANEtemp:
	    result = 1;
	    break;
	case M68am_FReg:
	    if (GetLocFReg(loc) <= MAXTEMPFLOATREG) {
		result = 1;
	    }
	    break;
	case M68am_DReg:
	    if (GetLocDReg(loc) <= MAXTEMPDATAREG) {
		result = 1;
	    }
	    break;
	case M68am_ARegDirect:
	case M68am_ARegIndirect:
	case M68am_ARegPostInc:
	case M68am_ARegPreDec:
	case M68am_ARegDisplace:
	case M68am_ARegDisplaceFIELD:
	case M68am_ARegDispIndx:
	    if (GetLocAReg(loc) <= MAXTEMPADDRREG) {
		result = 1;
	    }
	    break;
	default:
	    break;
	}
    }
    return result;
}

void
FreePerm(LocAMVia_t loc)
{
    if (loc) {
	if (isPermReg(loc)) {
	    switch (GetLocAM(loc)) {
		case M68am_DReg:
		TempDatas[GetLocDReg(loc)].status = M68rs_Available;
		TempDatas[GetLocDReg(loc)].holder = NULL;
		break;
	    case M68am_FReg:
		TempFloats[GetLocFReg(loc)].status = M68rs_Available;
		TempFloats[GetLocFReg(loc)].holder = NULL;
		break;
	    case M68am_FSANEtemp:
		if (GetLocSlot(loc)) {
		    Via(GetLocSlot(loc))->status = M68ss_free;
		}
		break;
	    default:		/* AReg is default. */
		TempAddrs[GetLocAReg(loc)].status = M68rs_Available;
		TempAddrs[GetLocAReg(loc)].holder = NULL;
		break;
	    }
	}
	if (GetLocSlot(loc)) {
	    Via(GetLocSlot(loc))->status = M68ss_free;
	}
    }
}

void
FreeTemp(LocAMVia_t loc)
/*
 * If the loc passed is a temporary data or address register, that register
 * is marked available.  Also, if the loc is a 68881 freg, or a spill slot,
 * it is marked available.
 */
{
    if (loc) {
	if (isTempReg(loc)) {
	    switch (GetLocAM(loc)) {
		case M68am_DReg:
		TempDatas[GetLocDReg(loc)].status = M68rs_Available;
		TempDatas[GetLocDReg(loc)].holder = NULL;
		break;
	    case M68am_FReg:
		TempFloats[GetLocFReg(loc)].status = M68rs_Available;
		TempFloats[GetLocFReg(loc)].holder = NULL;
		break;
	    case M68am_FSANEtemp:
		if (GetLocSlot(loc)) {
		    if (!Via(GetLocSlot(loc))->local)
			Via(GetLocSlot(loc))->status = M68ss_free;
		}
		break;
	    default:		/* AReg is default. */
		TempAddrs[GetLocAReg(loc)].status = M68rs_Available;
		TempAddrs[GetLocAReg(loc)].holder = NULL;
		break;
	    }
	}
	if (GetLocSlot(loc)) {
	    if (!Via(GetLocSlot(loc))->local)
		Via(GetLocSlot(loc))->status = M68ss_free;
	}
    }
}

void
FreeIt(LocAMVia_t loc)
{
    FreeTemp(loc);
    FreePerm(loc);
}

LocAMVia_t
ValidateReg(LocAMVia_t loc, InstListVia_t Codes)
/*
 * If the loc passed is a temporary data/addr/float register which is
 * currently spilled, then it is brought off the stack frame and restored to
 * its place.
 */
{
    LocAMVia_t                      result;
    result = loc;
    if (loc) {
	switch (GetLocAM(loc)) {
	case M68am_FReg:
	    if (GetLocFReg(loc) <= MAXTEMPFLOATREG) {
		if (GetLocStatus(loc) != M68os_valid) {
		    if (TempFloats[GetLocFReg(loc)].status == M68rs_Available) {
			UnSpillTemp(loc, Codes);
			TempFloats[GetLocFReg(loc)].status = M68rs_Used;
			TempFloats[GetLocFReg(loc)].holder = loc;
			TempFloats[GetLocFReg(loc)].waiting--;
		    } else {
			Gen68Error("VERYBAD float Reg being validated was not available !");
		    }
		}
	    }
	    break;
	case M68am_DReg:
	    if (GetLocDReg(loc) <= MAXTEMPDATAREG) {
		if (GetLocStatus(loc) != M68os_valid) {
		    if (TempDatas[GetLocDReg(loc)].status == M68rs_Available) {
			UnSpillTemp(loc, Codes);
			TempDatas[GetLocDReg(loc)].status = M68rs_Used;
			TempDatas[GetLocDReg(loc)].holder = loc;
			TempDatas[GetLocDReg(loc)].waiting--;
		    } else {
			Gen68Error("VERYBAD DReg being validated was not available !");
		    }
		}
	    }
	    break;
	case M68am_ARegDirect:
	case M68am_ARegIndirect:
	case M68am_ARegPostInc:
	case M68am_ARegPreDec:
	case M68am_ARegDisplace:
	case M68am_ARegDispIndx:
	    if (GetLocAReg(loc) <= MAXTEMPADDRREG) {
		if (GetLocStatus(loc) != M68os_valid) {
		    if (TempAddrs[GetLocAReg(loc)].status == M68rs_Available) {
			UnSpillTemp(loc, Codes);
			TempAddrs[GetLocAReg(loc)].status = M68rs_Used;
			TempAddrs[GetLocAReg(loc)].holder = loc;
			TempAddrs[GetLocAReg(loc)].waiting--;
		    } else {
			Gen68Error("VERYBAD AReg being validated was not available !");
		    }
		}
	    }
	    break;
	default:
	    break;
	}
    }
    return result;
}

int
AvailFloatReg(void)
/*
 * Returns the number of an available float register, if there is one
 * available.  This only applies for the 68881 mode, because there are an
 * arbitrary number of SANE temps which can be created on the stack frame. We
 * have two priorities here.  first, we want to return a reg which is
 * available if possible.  Second, whether we return an available reg or not,
 * we want to return the one with the fewest locations spilled (waiting).
 */
{
    int                             ndx;
    int                             loop;
    int                             fewest;
    fewest = -1;
    ndx = NextTempFloatReg;
    loop = 0;
    while (!loop) {
	if (TempFloats[ndx].status == M68rs_Available) {
	    if (fewest < 0) {
		fewest = ndx;
	    } else {
		if (TempFloats[ndx].waiting < TempFloats[fewest].waiting) {
		    fewest = ndx;
		}
	    }
	}
	ndx = (ndx + 1) % NUMOFTEMPFLOATREGS;
	if (ndx == NextTempFloatReg) {
	    loop = 1;
	}
    }
    if (fewest >= 0) {
	return fewest;
    }
    /*
     * OK. None are available, find the one with the fewest locs waiting.
     */

    fewest = NextTempFloatReg;
    ndx = NextTempFloatReg;
    loop = 0;
    while (!loop) {
	if (TempFloats[ndx].waiting < TempFloats[fewest].waiting) {
	    fewest = ndx;
	}
	ndx = (ndx + 1) % NUMOFTEMPFLOATREGS;
	if (ndx == NextTempFloatReg) {
	    loop = 1;
	}
    }
    return fewest;
}

int
AvailAddrPerm(void)
/*
 * Returns the number of an available perm addr register, if there is one
 * available.
 */
{
    int                             ndx;
    ndx = MAXTEMPADDRREG + 1;
    while (ndx <= 6) {
	if (ndx != FRAME)
	    if (ndx != 5)
		if (TempAddrs[ndx].status == M68rs_Available)
		    return ndx;
	ndx++;
    }
    return 0;
}

void
PopPerms(InstListVia_t Codes)
{
    /*
     * This function scans TempDatas (perms only) and determines if any perms
     * were used.  If so, it generates instructions to get them from the
     * stack, and inserts those instructions into Codes
     */
    int                             ndx;
    LocAMVia_t                      stkpop;
    stkpop = RawLocation();
    SetLocAM(stkpop, M68am_ARegPostInc);
    SetLocAReg(stkpop, 7);

    ndx = MAXTEMPDATAREG + 1;
    while (ndx <= 7) {
	if (TempDatas[ndx].count) {
	    GenInst(M68op_MOVE, M68sz_long, stkpop, BuildDRegLocation(ndx),
		    Codes);
	}
	ndx++;
    }
    ndx = MAXTEMPFLOATREG + 1;
    while (ndx <= 7) {
	if (TempFloats[ndx].count) {
	    GenInst(M68op_FMOVE, M68sz_extended, stkpop, BuildFRegLocation(ndx),
		    Codes);
	}
	ndx++;
    }
    ndx = MAXTEMPADDRREG + 1;
    while (ndx <= 6) {
	if (TempAddrs[ndx].count) {
	    GenInst(M68op_MOVE, M68sz_long, stkpop, BuildARegDirect(ndx),
		    Codes);
	}
	ndx++;
    }
}

void
PushPerms(void)
{
    /*
     * This function scans TempDatas (perms only) and determines if any perms
     * were used.  If so, it generates instructions to save them onto the
     * stack, and inserts those instructions immediately after LinkInst.
     */
    int                             ndx;
    LocAMVia_t                      stkpush;
    stkpush = RawLocation();
    SetLocAM(stkpush, M68am_ARegPreDec);
    SetLocAReg(stkpush, 7);

    ndx = MAXTEMPDATAREG + 1;
    while (ndx <= 7) {
	if (TempDatas[ndx].count) {
	    InsertInstruction(M68op_MOVE, M68sz_long, BuildDRegLocation(ndx),
			      stkpush, LinkInst);
	}
	ndx++;
    }
    ndx = MAXTEMPFLOATREG + 1;
    while (ndx <= 7) {
	if (TempFloats[ndx].count) {
	    InsertInstruction(M68op_FMOVE, M68sz_extended, BuildFRegLocation(ndx),
			      stkpush, LinkInst);
	}
	ndx++;
    }
    ndx = MAXTEMPADDRREG + 1;
    while (ndx <= 6) {
	if (TempAddrs[ndx].count) {
	    InsertInstruction(M68op_MOVE, M68sz_long, BuildARegDirect(ndx),
			      stkpush, LinkInst);
	}
	ndx++;
    }
}

int
AvailFloatPerm(void)
/*
 * Returns the number of an available perm float register, if there is one
 * available.
 */
{
    int                             ndx;
    ndx = MAXTEMPFLOATREG + 1;
    while (ndx <= 7) {
	if (TempFloats[ndx].status == M68rs_Available) {
	    return ndx;
	}
	ndx++;
    }
    return 0;
}

int
AvailDataPerm(void)
/*
 * Returns the number of an available perm data register, if there is one
 * available.
 */
{
    int                             ndx;
    ndx = MAXTEMPDATAREG + 1;
    while (ndx <= 7) {
	if (TempDatas[ndx].status == M68rs_Available) {
	    return ndx;
	}
	ndx++;
    }
    return 0;
}

int
AvailDataReg(void)
/*
 * Returns the number of an available data register, if there is one
 * available. We have two priorities here.  first, we want to return a reg
 * which is available if possible.  Second, whether we return an available
 * reg or not, we want to return the one with the fewest locations spilled
 * (waiting). Unfortunately, there is a third consideration.  Sometimes there
 * is a tie.  It occurs when there is more than one available register with
 * the same value for waiting.  How do we break the tie ?  The global var
 * NextTempDataReg is used to determine which is the next reg to be
 * allocated.
 */
{
    int                             ndx;
    int                             loop;
    int                             fewest;
    fewest = -1;
    ndx = NextTempDataReg;
    loop = 0;
    while (!loop) {
	if (TempDatas[ndx].status == M68rs_Available) {
	    if (fewest < 0) {
		fewest = ndx;
	    } else {
		if (TempDatas[ndx].waiting < TempDatas[fewest].waiting) {
		    fewest = ndx;
		}
	    }
	}
	ndx = (ndx + 1) % NUMOFTEMPDATAREGS;
	if (ndx == NextTempDataReg) {
	    loop = 1;
	}
    }
    if (fewest >= 0) {
	return fewest;
    }
    /*
     * OK. None are available, find the one with the fewest locs waiting.
     */

    fewest = NextTempDataReg;
    ndx = NextTempDataReg;
    loop = 0;
    while (!loop) {
	if (TempDatas[ndx].waiting < TempDatas[fewest].waiting) {
	    fewest = ndx;
	}
	ndx = (ndx + 1) % NUMOFTEMPDATAREGS;
	if (ndx == NextTempDataReg) {
	    loop = 1;
	}
    }
    return fewest;
}

int
AvailAddrReg(void)
/*
 * Returns the number of an available addr register, if there is one
 * available.
 */
{
    int                             ndx;
    int                             loop;
    int                             fewest;
    fewest = -1;
    ndx = NextTempAddrReg;
    loop = 0;
    while (!loop) {
	if (TempAddrs[ndx].status == M68rs_Available) {
	    if (fewest < 0) {
		fewest = ndx;
	    } else {
		if (TempAddrs[ndx].waiting < TempAddrs[fewest].waiting) {
		    fewest = ndx;
		}
	    }
	}
	ndx = (ndx + 1) % NUMOFTEMPADDRREGS;
	if (ndx == NextTempAddrReg) {
	    loop = 1;
	}
    }
    if (fewest >= 0) {
	return fewest;
    }
    /*
     * OK. None are available, find the one with the fewest locs waiting.
     */

    fewest = NextTempAddrReg;
    ndx = NextTempAddrReg;
    loop = 0;
    while (!loop) {
	if (TempAddrs[ndx].waiting < TempAddrs[fewest].waiting) {
	    fewest = ndx;
	}
	ndx = (ndx + 1) % NUMOFTEMPADDRREGS;
	if (ndx == NextTempAddrReg) {
	    loop = 1;
	}
    }
    return fewest;
}

LocAMVia_t
TempAddrReg(InstListVia_t Codes)
/*
 * Allocates and returns the location record for a temporary addr register.
 * If all the temp addr registers are full, then one is spilled onto the
 * stack to make room.
 */
{
    LocAMVia_t                      result;
    SpillSlotVia_t                  slot;
    NextTempAddrReg = AvailAddrReg();
    result = BuildARegDirect(NextTempAddrReg);
    if (TempAddrs[NextTempAddrReg].status != M68rs_Available) {
	slot = SpillTemp(TempAddrs[NextTempAddrReg].holder, Codes);
	TempAddrs[NextTempAddrReg].waiting++;
    }
    TempAddrs[NextTempAddrReg].holder = result;
    TempAddrs[NextTempAddrReg].status = M68rs_Used;
    TempAddrs[NextTempAddrReg].count++;
    NextTempAddrReg = (NextTempAddrReg + 1) % NUMOFTEMPADDRREGS;
    return result;
}

LocAMVia_t
TempFloatReg(InstListVia_t Codes)
/*
 * Allocates and returns the location record for a temporary float register.
 * If all the temp float registers are full, then one is spilled onto the
 * stack to make room.
 */
{
    /* Check for SANE mode here and handle it differently. */
    LocAMVia_t                      result;
    SpillSlotVia_t                  slot;
    if (gProject->itsOptions->useMC68881) {
	NextTempFloatReg = AvailFloatReg();
	result = BuildFRegLocation(NextTempFloatReg);
	if (TempFloats[NextTempFloatReg].status != M68rs_Available) {
	    slot = SpillTemp(TempFloats[NextTempFloatReg].holder, Codes);
	    TempFloats[NextTempFloatReg].waiting++;
	}
	TempFloats[NextTempFloatReg].holder = result;
	TempFloats[NextTempFloatReg].status = M68rs_Used;
	NextTempFloatReg = (NextTempFloatReg + 1) % NUMOFTEMPFLOATREGS;
    } else {
	slot = GetSpillSlot(10);
	result = Via(slot)->SlotLoc;
	SetLocSZ(result, 10);
	SetLocAM(result, M68am_FSANEtemp);
    }
    SetLocIsFloat(result, 1);
    return result;
}

LocAMVia_t
PermAddrReg(void)
/*
 * Allocates and returns the location record for a perm addr register.  If
 * all the perm addr registers are full, then the caller is out of luck.
 */
{
    LocAMVia_t                      result;
    int                             whichreg;
    whichreg = AvailAddrPerm();
    if (!whichreg) {
	return NULL;
    }
    result = BuildARegDirect(whichreg);
    TempAddrs[whichreg].holder = result;
    TempAddrs[whichreg].status = M68rs_Used;
    TempAddrs[whichreg].count++;
    return result;
}

LocAMVia_t
PermFloatReg(void)
/*
 * Allocates and returns the location record for a perm float register.  If
 * all the perm float registers are full, then the caller is out of luck.
 */
{
    LocAMVia_t                      result;
    int                             whichreg;
    whichreg = AvailFloatPerm();
    if (!whichreg) {
	return NULL;
    }
    result = BuildFRegLocation(whichreg);
    TempFloats[whichreg].holder = result;
    TempFloats[whichreg].status = M68rs_Used;
    TempFloats[whichreg].count++;
    return result;
}

LocAMVia_t
PermDataReg(void)
/*
 * Allocates and returns the location record for a perm data register.  If
 * all the perm data registers are full, then the caller is out of luck.
 */
{
    LocAMVia_t                      result;
    int                             whichreg;
    whichreg = AvailDataPerm();
    if (!whichreg) {
	return NULL;
    }
    result = BuildDRegLocation(whichreg);
    TempDatas[whichreg].holder = result;
    TempDatas[whichreg].status = M68rs_Used;
    TempDatas[whichreg].count++;
    return result;
}

LocAMVia_t
TempDataReg(InstListVia_t Codes)
/*
 * Allocates and returns the location record for a temporary data register.
 * If all the temp data registers are full, then one is spilled onto the
 * stack to make room.
 */
{
    LocAMVia_t                      result;
    SpillSlotVia_t                  slot;
    NextTempDataReg = AvailDataReg();
    result = BuildDRegLocation(NextTempDataReg);
    if (TempDatas[NextTempDataReg].status != M68rs_Available) {
	slot = SpillTemp(TempDatas[NextTempDataReg].holder, Codes);
	TempDatas[NextTempDataReg].waiting++;
    }
    TempDatas[NextTempDataReg].holder = result;
    TempDatas[NextTempDataReg].status = M68rs_Used;
    TempDatas[NextTempDataReg].count++;
    NextTempDataReg = (NextTempDataReg + 1) % NUMOFTEMPDATAREGS;
    return result;
}

LocAMVia_t
DictateTempFloat(LocAMVia_t operand, InstListVia_t Codes)
/*
 * Ensures the the loc passed is a temp float register, generating a move
 * instruction to make it so, if necessary.
 */
{
    LocAMVia_t                      result;
    if (gProject->itsOptions->useMC68881) {
	if (GetLocAM(operand) == M68am_FReg) {
	    if (isTempReg(operand)) {
		return operand;
	    }
	}
    } else {
	if (GetLocAM(operand) == M68am_FSANEtemp) {
	    return operand;
	}
    }
    if (isTempReg(operand)) {
	FreeTemp(operand);
    }
    result = TempFloatReg(Codes);
    Genmove(GetLocSZ(operand), operand, result, Codes);
    return result;
}

LocAMVia_t
DictateAnyFReg(LocAMVia_t operand, InstListVia_t Codes)
/*
 * Ensures the the loc passed is a float register, allocating a temp to make
 * it so, if necessary.
 */
{
    if (operand) {
	if (GetLocAM(operand) != M68am_FReg) {
	    operand = DictateTempFloat(operand, Codes);
	}
    }
    return operand;
}

LocAMVia_t
DictateAnyDReg(LocAMVia_t operand, InstListVia_t Codes)
/*
 * Ensures the the loc passed is an data register, allocating a temp to make
 * it so, if necessary.
 */
{
    if (operand) {
	if (GetLocAM(operand) != M68am_DReg) {
	    operand = DictateTempDReg(operand, Codes);
	}
    }
    return operand;
}

LocAMVia_t
DictateAnyAReg(LocAMVia_t operand, InstListVia_t Codes)
/*
 * Ensures the the loc passed is an addr register, allocating a temp to make
 * it so, if necessary.
 */
{
    if (operand) {
	if (GetLocAM(operand) != M68am_ARegDirect) {
	    operand = DictateTempAReg(operand, Codes);
	}
    }
    return operand;
}

LocAMVia_t
DictateTempAReg(LocAMVia_t operand, InstListVia_t Codes)
/*
 * Ensures the the loc passed is a temp addr register, generating a move
 * instruction to make it so, if necessary.
 */
{
    LocAMVia_t                      result;
    ValidateReg(operand, Codes);
    if (GetLocAM(operand) == M68am_ARegDirect) {
	if (isTempReg(operand)) {
	    return operand;
	}
    }
    if (isTempReg(operand)) {
	FreeTemp(operand);
    }
    result = TempAddrReg(Codes);
    SetLocSZ(result, GetLocSZ(operand));
    Genmove(GetLocSZ(operand), operand, result, Codes);
    return result;
}

LocAMVia_t
DictateTempDReg(LocAMVia_t operand, InstListVia_t Codes)
/*
 * Ensures the the loc passed is a temp data register, generating a move
 * instruction to make it so, if necessary.
 */
{
    LocAMVia_t                      result;
    if (GetLocAM(operand) == M68am_DReg) {
	if (isTempReg(operand)) {
	    return operand;
	}
    }
    if (isTempReg(operand)) {
	FreeTemp(operand);
    }
    result = TempDataReg(Codes);
    SetLocSZ(result, GetLocSZ(operand));
    if (GetLocSZ(operand) == M68sz_none) {
	Gen68Error("Missing size in dictatetempdreg");
    }
    Genmove(GetLocSZ(operand), operand, result, Codes);
    /*
     * TODO Here, do we need to sign extend or something if this is not a
     * long ?
     */
    return result;
}

void
SaveDataRegisters(InstListVia_t Codes)
/*
 * Spills all used data registers, leaving them all free for use. This
 * routine is used before all library calls.
 */
{
    int                             ndx;

    ndx = 0;
    while (ndx <= MAXTEMPDATAREG) {
	if (TempDatas[ndx].status != M68rs_Available) {
	    SpillTemp(TempDatas[ndx].holder, Codes);
	    TempDatas[ndx].status = M68rs_Available;
	    TempDatas[ndx].holder = NULL;
	}
	ndx++;
    }
}

void
SaveTheRegisters(InstListVia_t Codes)
/*
 * Spills all used registers, leaving them all free for use.  This routine is
 * used before all function calls.
 */
{
    int                             ndx;

    ndx = 0;
    while (ndx <= MAXTEMPADDRREG) {
	if (TempAddrs[ndx].status != M68rs_Available) {
	    SpillTemp(TempAddrs[ndx].holder, Codes);
	    TempAddrs[ndx].status = M68rs_Available;
	    TempAddrs[ndx].holder = NULL;
	}
	ndx++;
    }
    ndx = 0;
    while (ndx <= MAXTEMPFLOATREG) {
	if (TempFloats[ndx].status != M68rs_Available) {
	    SpillTemp(TempFloats[ndx].holder, Codes);
	    TempFloats[ndx].status = M68rs_Available;
	    TempFloats[ndx].holder = NULL;
	}
	ndx++;
    }
    ndx = 0;
    while (ndx <= MAXTEMPDATAREG) {
	if (TempDatas[ndx].status != M68rs_Available) {
	    SpillTemp(TempDatas[ndx].holder, Codes);
	    TempDatas[ndx].status = M68rs_Available;
	    TempDatas[ndx].holder = NULL;
	}
	ndx++;
    }
}
