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
 * This file contains routines for initializations of data.
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


#pragma segment Initializers

SYMVia_t
NextSym(SymListVia_t tbl, SYMVia_t cur)
{
    return TableGetNum(tbl, Via(cur)->IndexInserted + 1);
}

SYMVia_t
LastSym(SymListVia_t tbl)
{
    return TableGetNum(tbl, Via(tbl)->count);
}

void
ImageStruct(SymListVia_t tbl, SYMVia_t member, ParseTreeVia_t init, InstListVia_t Codes)
{
    /*
     * Both member and init are probably lists. We need to be at the tail of
     * both.
     */
    if (init && member) {
	if ((Via(init)->kind == PTF_initializer_list) && (NextSym(tbl, member))) {
	    ImageStruct(tbl, NextSym(tbl, member), Via(init)->b, Codes);
	    ImageInit(Via(member)->TP, Via(init)->a, Codes);
	} else if ((Via(init)->kind == PTF_initializer_list) && (!NextSym(tbl, member))) {
	    ImageStruct(tbl, member, Via(init)->b, Codes);
	    ImageInit(Via(member)->TP, Via(init)->a, Codes);
	} else if ((Via(init)->kind != PTF_initializer_list) && (!NextSym(tbl, member))) {
	    ImageInit(Via(member)->TP, init, Codes);
	} else if ((Via(init)->kind != PTF_initializer_list) && (NextSym(tbl, member))) {
	    ImageStruct(tbl, NextSym(tbl, member), init, Codes);
	}
    }
}

void
ImageArray(TypeRecordVia_t typerec, ParseTreeVia_t init, InstListVia_t Codes)
{
    if (init) {
	if ((Via(init)->kind == PTF_initializer_list) && !isArrayType(typerec)) {
	    ImageArray(typerec, Via(init)->b, Codes);
	    ImageInit(typerec, Via(init)->a, Codes);
	} else if ((Via(init)->kind == PTF_initializer_list) && isArrayType(typerec)) {
	    ImageArray(GetTPBase(typerec), init, Codes);
	} else {
	    ImageInit(typerec, init, Codes);
	}
    }
}

LocAMVia_t
BuildFoldK(FoldValue_t val)
{
    if (val.isint) {
	return BuildAbsolute(val.intval);
    } else if (val.isstring) {
	return BuildLabelLoc(GetLocLabel(Via(Via(val.init)->data.SLit)->M68kDef.Loc));
    } else if (val.issymb) {
	return Via(val.thesymbol)->M68kDef.Loc;
    } else if (val.isoffset) {
	return BuildLabelOffset(GetLocLabel(Via(val.thesymbol)->M68kDef.Loc), val.intval);
    } else
	return BuildAbsolute(0);
}

int
ImageInit(TypeRecordVia_t typerec, ParseTreeVia_t init, InstListVia_t Codes)
{
    FoldValue_t                     val;
    int                             ndx;
    ParseTreeVia_t                  tmptree;
    int                             oldcount;
    union {
	float                           val1;
	double                          val2;
	long double                     val3;
	char                            bytes[10];
    }                               floatbytes;
    if (init && typerec) {
	ConstExprValue(init, &val);
	switch (GetTPKind(typerec)) {
	case TRC_typedef:
	    return ImageInit(GetTPBase(typerec), init, Codes);
	    break;
	case TRC_char:
	    if (Via(init)->kind == PTF_string_literal) {
		InitDataAccIndex += strlen((Via(Via(init)->data.SLit)->name));
		GenOneLit(Via(init)->data.SLit, Codes);
	    } else {
		GenInst(M68op_DC, M68sz_byte, BuildFoldK(val), NULL, Codes);
		InitDataAccIndex++;
	    }
	    break;
	case TRC_short:
	    GenInst(M68op_DC, M68sz_word, BuildFoldK(val), NULL, Codes);
	    InitDataAccIndex += 2;
	    break;
	case TRC_int:
	    if (gProject->itsOptions->int2byte) {
		GenInst(M68op_DC, M68sz_word, BuildFoldK(val), NULL, Codes);
		InitDataAccIndex += 2;
	    } else {
		GenInst(M68op_DC, M68sz_long, BuildFoldK(val), NULL, Codes);
		InitDataAccIndex += 4;
	    }
	    break;
	case TRC_long:
	    GenInst(M68op_DC, M68sz_long, BuildFoldK(val), NULL, Codes);
	    InitDataAccIndex += 4;
	    break;
	case TRC_enum:
	    if (gProject->itsOptions->int2byte) {
		GenInst(M68op_DC, M68sz_word, BuildFoldK(val), NULL, Codes);
		InitDataAccIndex += 2;
	    } else {
		GenInst(M68op_DC, M68sz_long, BuildFoldK(val), NULL, Codes);
		InitDataAccIndex += 4;
	    }
	    break;
	case TRC_pointer:
#ifdef Undefined
	    if (Via(init)->kind == PTF_string_literal) {
		GenInst(M68op_DC, M68sz_long, BuildLabelLoc(GetLocLabel(Via(Via(init)->data.SLit)->M68kDef.Loc)), NULL, Codes);
		InitDataAccIndex += 4;
	    } else if (Via(init)->kind == PTF_identifier) {
		GenInst(M68op_DC, M68sz_long, Via(Via(init)->data.thesymbol)->M68kDef.Loc, NULL, Codes);
		InitDataAccIndex += 4;
	    } else {
		GenInst(M68op_DC, M68sz_long, BuildFoldK(val), NULL, Codes);
		InitDataAccIndex += 4;
	    }
#endif
	    GenInst(M68op_DC, M68sz_long, BuildFoldK(val), NULL, Codes);
	    InitDataAccIndex += 4;
	    break;
	case TRC_float:
	    if (val.isint) {
		floatbytes.val1 = val.intval;
	    } else {
		floatbytes.val1 = val.realval;
	    }
	    ndx = 0;
	    /*
	     * Problem with constants here - depends on whether or not SANE
	     * is running right now. TODO DANGER
	     */
	    while (ndx < SizeOfFloat) {
		GenInst(M68op_DC, M68sz_byte, BuildAbsolute(floatbytes.bytes[ndx]), NULL, Codes);
		InitDataAccIndex++;
		ndx++;
	    }
	    break;
	case TRC_double:
	    if (val.isint) {
		floatbytes.val2 = val.intval;
	    } else {
		floatbytes.val2 = val.realval;
	    }
	    ndx = 0;
	    /*
	     * Problem with constants here - depends on whether or not SANE
	     * is running right now. TODO DANGER
	     */
	    while (ndx < SizeOfDouble) {
		GenInst(M68op_DC, M68sz_byte, BuildAbsolute(floatbytes.bytes[ndx]), NULL, Codes);
		InitDataAccIndex++;
		ndx++;
	    }
	    break;
	case TRC_longdouble:
	    if (val.isint) {
		floatbytes.val3 = val.intval;
	    } else {
		floatbytes.val3 = val.realval;
	    }
	    ndx = 0;
	    /*
	     * Problem with constants here - depends on whether or not SANE
	     * is running right now. TODO DANGER
	     */
	    while (ndx < SizeOfLongDouble) {
		GenInst(M68op_DC, M68sz_byte, BuildAbsolute(floatbytes.bytes[ndx]), NULL, Codes);
		InitDataAccIndex++;
		ndx++;
	    }
	    break;
	case TRC_struct:
	    oldcount = InitDataAccIndex;
	    tmptree = init;
	    if (Via(tmptree)->kind == PTF_multi_initializer) {
		tmptree = Via(init)->a;
	    }
	    ImageStruct(GetTPMembers(typerec), FirstSym(GetTPMembers(typerec)), tmptree, Codes);
	    if (((InitDataAccIndex - oldcount) < GetTPSize(typerec))) {
		ndx = 0;
		while (ndx < (GetTPSize(typerec) - (InitDataAccIndex - oldcount))) {
		    GenInst(M68op_DC, M68sz_byte, BuildAbsolute(0), NULL, Codes);
		    InitDataAccIndex++;
		    ndx++;
		}
	    }
	    break;
	case TRC_union:
	    tmptree = init;
	    /* Should this change be mandatory ?! */
	    if (Via(tmptree)->kind == PTF_multi_initializer) {
		tmptree = Via(init)->a;
	    }
	    ImageInit(Via(LastSym(GetTPMembers(typerec)))->TP, tmptree, Codes);
	    break;
	case TRC_array:
	    oldcount = InitDataAccIndex;
	    tmptree = init;
	    if (Via(tmptree)->kind == PTF_multi_initializer) {
		tmptree = Via(init)->a;
	    }
	    ImageArray(GetTPBase(typerec), tmptree, Codes);
	    if (GetTPSize(typerec)) {
		if (((InitDataAccIndex - oldcount) < GetTPSize(typerec))) {
		    ndx = 0;
		    while (ndx < (GetTPSize(typerec) - (InitDataAccIndex - oldcount))) {
			GenInst(M68op_DC, M68sz_byte, BuildAbsolute(0), NULL, Codes);
			InitDataAccIndex++;
			ndx++;
		    }
		} else if (((InitDataAccIndex - oldcount) > GetTPSize(typerec))) {
		    DeclError("Too much data in initializer");
		}
	    } else {
		SetTPFlags(typerec, GetTPFlags(typerec) & (~INCOMPLETEMASK));
		SetTPSize(typerec, InitDataAccIndex - oldcount);
	    }
	    break;
	}
    }
    return InitDataAccIndex;
}
