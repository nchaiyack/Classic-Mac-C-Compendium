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
 * This file implements operations on LocAM records.
 * 
 */

#include "conditcomp.h"
#include <stdio.h>
#include <string.h>
#include "structs.h"


#pragma segment LocAM

#include "CodeGen.h"

static LocAMVia_t               LocPile;
static LocAMVia_t               StorageLocPile;

struct LocAM_S {
    enum AddressingMode68           AM;
    enum OperandState68             status;
#ifdef INLINEASM
    struct ea P__H                 *OtherFormat;
#endif
    SpillSlotVia_t                  slot;
    struct TwoShorts                FieldBits;
    enum Size68                     SZ;
    char                            AReg;
    char                            DReg;
    char                            FReg;	/* Are all three of these reg
						 * number fields necessary ? */
    char                            isFLOAT;
    unsigned long                   Constant;	/* Is there any instance
						 * wherein the Label AND the
						 * Constant are both used ? */
    LabSYMVia_t                     Label;
    LocAMVia_t                      next;
};

LocAMVia_t
RawLocation(void)
{
    LocAMVia_t                      raw;
    raw = Ealloc(sizeof(LocAM_t));
    Via(raw)->AM = 0;
    Via(raw)->status = M68os_valid;
#ifdef INLINEASM
    Via(raw)->OtherFormat = NULL;
#endif
    Via(raw)->DReg = NOREG68;
    Via(raw)->AReg = NOREG68;
    Via(raw)->FReg = NOREG68;
    Via(raw)->isFLOAT = 0;
    Via(raw)->SZ = M68sz_none;
    Via(raw)->slot = NULL;
    Via(raw)->Constant = 0;
    Via(raw)->Label = NULL;
    Via(raw)->next = LocPile;
    LocPile = raw;
    return raw;
}

enum AddressingMode68
GetLocAM(LocAMVia_t loc)
{
    assert(loc);
    return Via(loc)->AM;
}

int
isConstantLoc(LocAMVia_t loc)
{
	if (Via(loc)->AM == M68am_Immediate) return 1;
	return 0;
}

enum Size68
GetLocSZ(LocAMVia_t loc)
{
    assert(loc);
    return Via(loc)->SZ;
}

enum OperandState68
GetLocStatus(LocAMVia_t loc)
{
    assert(loc);
    return Via(loc)->status;
}

void
SetLocStatus(LocAMVia_t loc, enum OperandState68 s)
{
    assert(loc);
    Via(loc)->status = s;
}

SpillSlotVia_t
GetLocSlot(LocAMVia_t loc)
{
    assert(loc);
    return Via(loc)->slot;
}

void
SetLocSlot(LocAMVia_t loc, SpillSlotVia_t s)
{
    assert(loc);
    Via(loc)->slot = s;
}

char
GetLocAReg(LocAMVia_t loc)
{
    assert(loc);
    return Via(loc)->AReg;
}

void
SetLocAReg(LocAMVia_t loc, char a)
{
    assert(loc);
    Via(loc)->AReg = a;
}

char
GetLocDReg(LocAMVia_t loc)
{
    assert(loc);
    return Via(loc)->DReg;
}

char
GetLocFReg(LocAMVia_t loc)
{
    assert(loc);
    return Via(loc)->FReg;
}

LabSYMVia_t
GetLocLabel(LocAMVia_t loc)
{
    assert(loc);
    return Via(loc)->Label;
}

unsigned long
GetLocConstant(LocAMVia_t loc)
{
    assert(loc);
    return Via(loc)->Constant;
}

void
SetLocConstant(LocAMVia_t loc, unsigned long c)
{
    assert(loc);
    Via(loc)->Constant = c;
}

int
LocIsFloat(LocAMVia_t loc)
{
    assert(loc);
    return Via(loc)->isFLOAT;
}

void
SetLocIsFloat(LocAMVia_t loc, int val)
{
    assert(loc);
    Via(loc)->isFLOAT = val;
}


struct TwoShorts
GetLocFieldBits(LocAMVia_t loc)
{
    assert(loc);
    return Via(loc)->FieldBits;
}

void
SetLocAM(LocAMVia_t loc, enum AddressingMode68 AM)
{
    assert(loc);
    Via(loc)->AM = AM;
}

void
SetLocSZ(LocAMVia_t loc, enum Size68 SZ)
{
    assert(loc);
    Via(loc)->SZ = SZ;
}

int
SameLocation(LocAMVia_t a, LocAMVia_t b)
{
    /* Compares two locations */
    /* If the AM, [DAF]Reg, SZ, Constant, and Label all match... */
    if (!(a && b)) {
	return 0;
    }
    if (a == b)
	return 1;

    if (Via(a)->AM != Via(b)->AM) {
	return 0;
    }
    switch (Via(a)->AM) {
    case M68am_DReg:
	if (Via(a)->DReg == Via(b)->DReg) {
	    return 1;
	} else {
	    return 0;
	}
	break;
    case M68am_Label:
	if (Via(a)->Label == Via(b)->Label) {
	    return 1;
	} else {
	    return 0;
	}
	break;
    default:			/* Some AReg mode... */
	if (Via(a)->AReg == Via(b)->AReg) {
	    if (Via(a)->Constant == Via(b)->Constant) {
		return 1;
	    } else {
		return 0;
	    }
	} else {
	    return 0;
	}
	break;
    }
    return 0;
}

/* Location builders */

LocAMVia_t
BuildFRegLocation(int reg)
{
    LocAMVia_t                      result;
    result = NULL;
    if (reg >= 0 && reg <= 7) {
	result = RawLocation();
	Via(result)->AM = M68am_FReg;
	Via(result)->SZ = M68sz_double;
	Via(result)->FReg = reg;
	Via(result)->status = M68os_valid;
    } else {
	Gen68Error("Illegal register number for Freg");
    }
    return result;
}

LocAMVia_t
BuildDRegLocation(int reg)
{
    LocAMVia_t                      result;
    result = NULL;
    if (reg >= 0 && reg <= 7) {
	result = RawLocation();
	Via(result)->AM = M68am_DReg;
	Via(result)->DReg = reg;
	Via(result)->status = M68os_valid;
    } else {
	Gen68Error("Illegal register number for Dreg");
    }
    return result;
}

LocAMVia_t
BuildAbsolute(long constant)
{
    LocAMVia_t                      result;
    result = RawLocation();
    Via(result)->AM = M68am_AbsLong;
    Via(result)->Constant = constant;
    Via(result)->SZ = M68sz_none;
    return result;
}

LocAMVia_t
BuildImmediate(int constant, enum Size68 SZ)
{
    LocAMVia_t                      result;
    result = RawLocation();
    Via(result)->AM = M68am_Immediate;
    Via(result)->Constant = constant;
    Via(result)->SZ = SZ;
    return result;
}

LocAMVia_t
BuildLargeGlobal(LabSYMVia_t displ)
{
    LocAMVia_t                      result;
    result = RawLocation();
    Via(result)->AM = M68am_LargeGlobal;
    Via(result)->Label = displ;
    Via(result)->status = M68os_valid;	/* irrelevant */
    return result;
}

LocAMVia_t
BuildARegLabelDisplace(int reg, LabSYMVia_t displ)
{
    LocAMVia_t                      result;
    result = NULL;
    if (reg >= 0 && reg <= 7) {
	result = RawLocation();
	Via(result)->AM = M68am_ARegLabelDisplace;
	Via(result)->Label = displ;
	Via(result)->AReg = reg;
	Via(result)->status = M68os_valid;
    } else {
	Gen68Error("Illegal register number for Areg");
    }
    return result;
}

LocAMVia_t
BuildARegDisplaceField(int reg, int displ, struct TwoShorts bits)
{
    LocAMVia_t                      result;
    result = NULL;
    if (reg >= 0 && reg <= 7) {
	result = RawLocation();
	Via(result)->AM = M68am_ARegDisplaceFIELD;
	Via(result)->Constant = displ;
	Via(result)->FieldBits = bits;
	Via(result)->AReg = reg;
	Via(result)->status = M68os_valid;
    } else {
	Gen68Error("Illegal register number for Areg");
    }
    return result;
}

LocAMVia_t
BuildARegDisplace(int reg, int displ)
{
    LocAMVia_t                      result;
    result = NULL;
    if (reg >= 0 && reg <= 7) {
	result = RawLocation();
	Via(result)->AM = M68am_ARegDisplace;
	Via(result)->Constant = displ;
	Via(result)->AReg = reg;
	Via(result)->status = M68os_valid;
    } else {
	Gen68Error("Illegal register number for Areg");
    }
    return result;
}

LocAMVia_t
BuildARegPreDec(int reg)
{
    LocAMVia_t                      result;
    result = NULL;
    if (reg >= 0 && reg <= 7) {
	result = RawLocation();
	Via(result)->AM = M68am_ARegPreDec;
	Via(result)->AReg = reg;
	Via(result)->status = M68os_valid;
    } else {
	Gen68Error("Illegal register number for Areg");
    }
    return result;
}

LocAMVia_t
BuildARegPostInc(int reg)
{
    LocAMVia_t                      result;
    result = NULL;
    if (reg >= 0 && reg <= 7) {
	result = RawLocation();
	Via(result)->AM = M68am_ARegPostInc;
	Via(result)->AReg = reg;
	Via(result)->status = M68os_valid;
    } else {
	Gen68Error("Illegal register number for Areg");
    }
    return result;
}

LocAMVia_t
BuildARegIndirect(int reg)
{
    LocAMVia_t                      result;
    result = NULL;
    if (reg >= 0 && reg <= 7) {
	result = RawLocation();
	Via(result)->AM = M68am_ARegIndirect;
	Via(result)->AReg = reg;
	Via(result)->status = M68os_valid;
    } else {
	Gen68Error("Illegal register number for Areg");
    }
    return result;
}

LocAMVia_t
BuildARegDirect(int reg)
{
    LocAMVia_t                      result;
    result = NULL;
    if (reg >= 0 && reg <= 7) {
	result = RawLocation();
	Via(result)->AM = M68am_ARegDirect;
	Via(result)->AReg = reg;
	Via(result)->status = M68os_valid;
	Via(result)->SZ = M68sz_long;	/* A default, since all pointers are
					 * 4 bytes. */
    } else {
	Gen68Error("Illegal register number for Areg");
    }
    return result;
}

LocAMVia_t
BuildAutoLoc(int offset, int size)
/*
 * Builds a location record addressing an offset from the frame pointer,
 * specified by the offset given.
 */
{
    LocAMVia_t                      result;
    result = NULL;
    result = RawLocation();
    Via(result)->AM = M68am_ARegDisplace;
    Via(result)->AReg = FRAME;
    Via(result)->Constant = offset;
    Via(result)->SZ = size;
    return result;
}

LocAMVia_t
BuildPCLabelDisplace(LabSYMVia_t name)
{
    LocAMVia_t                      result;
    result = RawLocation();
    Via(result)->AM = M68am_PCLabelDisplace;
    Via(result)->SZ = M68sz_long;
    Via(result)->Label = name;
    return result;
}

LocAMVia_t
BuildImmedLabelLoc(LabSYMVia_t name)
/*
 * Given a label record, builds a location record referencing that label as
 * an immediate, and returns it. Generally this loc references a string
 * literal.
 */
{
    LocAMVia_t                      result;
    result = RawLocation();
    Via(result)->AM = M68am_Immediate;
    Via(result)->SZ = M68sz_long;
    Via(result)->Label = name;
    return result;
}

LocAMVia_t
BuildLabelOffset(LabSYMVia_t name, long c)
/*
 * Given a label record, builds a location record referencing that label as
 * an address.  Possibly dangerous for the Mac version.
 */
{
    LocAMVia_t                      result;
    result = RawLocation();
    Via(result)->AM = M68am_LabelOffset;
    Via(result)->Label = name;
    Via(result)->Constant = c;
    return result;
}

LocAMVia_t
BuildLabelLoc(LabSYMVia_t name)
/*
 * Given a label record, builds a location record referencing that label as
 * an address.  Possibly dangerous for the Mac version.
 */
{
    LocAMVia_t                      result;
    result = RawLocation();
    Via(result)->AM = M68am_Label;
    Via(result)->Label = name;
    return result;
}

void
KillLocation(LocAMVia_t loc)
{
    if (loc) {
#ifdef INLINEASM
	if (Via(loc)->OtherFormat)
	    Efree(Via(loc)->OtherFormat);
#endif
	Efree(loc);
    }
}

void
KillSomeLocations(void)
{
    LocAMVia_t                      cur;
    LocAMVia_t                      nxt;
    cur = LocPile;
    while (cur) {
	nxt = Via(cur)->next;
	KillLocation(cur);
	cur = nxt;
    }
    LocPile = NULL;
}

static void
KillGlobalLocations(void)
{
    LocAMVia_t                      cur;
    LocAMVia_t                      nxt;
    cur = StorageLocPile;
    while (cur) {
	nxt = Via(cur)->next;
	KillLocation(cur);
	cur = nxt;
    }
    StorageLocPile = NULL;
}

void
KillAllLocations(void)
{
    KillSomeLocations();
    KillGlobalLocations();
}

void
InitLocAM(void)
{
    LocPile = NULL;
    StorageLocPile = NULL;
}

void
MakeLocGlobal(LocAMVia_t loc)
{
    /* Move loc from LocPile to StorageLocPile */
    LocAMVia_t                      tmp, prev = NULL;
    tmp = LocPile;
    while (tmp != LocPile) {
	prev = tmp;
	tmp = Via(tmp)->next;
    }
    if (prev) {
	Via(prev)->next = Via(tmp)->next;
    } else {
	LocPile = Via(LocPile)->next;
    }
    Via(tmp)->next = StorageLocPile;
    StorageLocPile = tmp;
}
