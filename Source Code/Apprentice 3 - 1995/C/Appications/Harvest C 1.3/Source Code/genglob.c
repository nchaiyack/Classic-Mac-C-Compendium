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
 * global variables and functions.
 * 
 * 
 * 
 */


#include "conditcomp.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "structs.h"

#include "CHarvestDoc.h"
#include "CHarvestOptions.h"

extern CHarvestDoc *gProject;

SYMVia_t CurrentFunctionPascal;
long CurrentPascalArgs;

#pragma segment GenGlob


void
GenInitGlobal(SYMVia_t symbol, InstListVia_t Codes)
{
    InitDataAccIndex = 0;
    ImageInit(GetSymTP(symbol), Via(symbol)->Definition.Initializer, Codes);
}

void
GenPLit(char *bytes, InstListVia_t Codes)
/*
 * Generates the DC instructions to generate a pascal string literal.
 */
{
    int                             ndx;
    int                             len;
    ndx = 0;
    len = strlen(bytes);
    GenInst(M68op_DC, M68sz_byte, BuildAbsolute(len), NULL, Codes);
    while (ndx < len) {
	GenInst(M68op_DC, M68sz_byte,
		BuildAbsolute((bytes)[ndx]), NULL, Codes);
	ndx++;
    }
    /* MPW says that pascal string lits should be null terminated as well */
    GenInst(M68op_DC, M68sz_byte, BuildAbsolute(0), NULL, Codes);

}

void
GenLit(char *bytes, InstListVia_t Codes)
/*
 * Generates the DC instructions to generate a string literal.
 */
{
    int                             ndx;
    ndx = 0;
    while ((bytes)[ndx]) {
	GenInst(M68op_DC, M68sz_byte,
		BuildAbsolute((bytes)[ndx]), NULL, Codes);
	ndx++;
    }
    GenInst(M68op_DC, M68sz_byte, BuildAbsolute(0), NULL, Codes);

}

void
GenOneLit(SYMVia_t cursym, InstListVia_t Codes)
{
    if (!(GetSymFlags(cursym) & CODEOUTPUT)) {
	GenInst(M68op_DSEG, M68sz_none, NULL, NULL, Codes);
	SetSymFlags(cursym, GetSymFlags(cursym) | CODEOUTPUT);
	GenStringLabel(GetLocLabel(Via(cursym)->M68kDef.Loc), Codes);
#ifdef OLDMEM
	HLock((Handle) cursym);
#endif
	if (GetSymFlags(cursym) & ISPASCALSTRING)
	    GenPLit(Via(cursym)->name, Codes);
	else
	    GenLit(Via(cursym)->name, Codes);
#ifdef OLDMEM
	HUnlock((Handle) cursym);
#endif
    }
}

void
GenFloatLit(char *bytes, InstListVia_t Codes)
/*
 * Generates the Dc instructions for a floating point literal.
 */
{
    int                             ndx;
    ndx = 0;
    while (ndx < SizeOfLongDouble) {
	GenInst(M68op_DC, M68sz_byte,
		BuildAbsolute((long) bytes[ndx]), NULL, Codes);
	ndx++;
    }
}

void
AddStatic(SYMVia_t symbol)
{
    SYMVia_t                        copy;
    Via(symbol)->M68kDef.Loc = BuildLabelLoc(MakeEccLabel(NextEccLabel++));
    SetLocSZ(Via(symbol)->M68kDef.Loc, M68_TypeSize(GetSymTP(symbol)));
#ifdef OLDMEM
    HLock((Handle) symbol);
#endif
    copy = TableAdd(StaticTable, Via(symbol)->name);
    SetSymTP(copy, GetSymTP(symbol));
    Via(copy)->M68kDef.Loc = Via(symbol)->M68kDef.Loc;
    #ifdef OLDMEM
    HUnlock((Handle) symbol);
    #endif
}

void
GenFloatLits(InstListVia_t Codes)
/* Loop to generate all the floating point literals. */
{
    FloatLitVia_t                   cur;
    cur = FloatList;
    while (cur) {
	if (!(Via(cur)->FloatFlags & CODEOUTPUT)) {
	    if (gProject->itsOptions->useMC68881) {
		    GenInst(M68op_DSEG, M68sz_none, NULL, NULL, Codes);
	    }
	    Via(cur)->FloatFlags |= CODEOUTPUT;
	    GenInst(M68op_EVEN, M68sz_none, NULL, NULL, Codes);
	    GenStringLabel(GetLocLabel(Via(cur)->Loc), Codes);
	    #ifdef OLDMEM
	    HLock((Handle) cur);
	    #endif
	    GenFloatLit(Via(cur)->val.bytes, Codes);
	    #ifdef OLDMEM
	    HUnlock((Handle) cur);
	    #endif
	}
	cur = Via(cur)->next;
    }
}

int
AllocateParameters(SymListVia_t table, int argoff)
/*
 * Assigns location records to all the parameters of a function. Func
 * parameters are located at positive offsets from the frame pointer.
 */
{
    int                             offset;
    SYMVia_t                        cursym;
    int                             ndx;
    if (!table)
	return 0;
    ndx = 1;
    while (ndx <= Via(table)->count) {
	cursym = TableGetNum(table, ndx++);
	if (cursym) {
	    offset = argoff;
	    if (GetTPSize(GetSymTP(cursym)) == 1) {
		offset += 3;
	    }
	    if (GetTPSize(GetSymTP(cursym)) == 2) {
		offset += 2;
	    }
	    Via(cursym)->M68kDef.Loc = BuildAutoLoc(offset,
					    M68_TypeSize(GetSymTP(cursym)));
	    argoff += GetTPSize(GetSymTP(cursym));
	    if ((GetTPSize(GetSymTP(cursym))) == 1)
		argoff += 3;
	    if ((GetTPSize(GetSymTP(cursym))) == 2)
		argoff += 2;
	    if (argoff % 2) {
		argoff++;
	    }
	}
    }
    return argoff;
}

int
AllocatePascalParameters(SymListVia_t table, int argoff)
/*
 * Assigns location records to all the parameters of a pascal function. Func
 * parameters are located at positive offsets from the frame pointer.
 */
{
    int                             offset;
    SYMVia_t                        cursym;
    int                             ndx;
    if (!table)
	return 0;
    ndx = Via(table)->count;
    while (ndx >= 1) {
	cursym = TableGetNum(table, ndx--);
	if (cursym) {
	    offset = argoff;
	    if (GetTPSize(GetSymTP(cursym)) == 1) {
		offset += 1;
	    }
	    Via(cursym)->M68kDef.Loc = BuildAutoLoc(offset,
					    M68_TypeSize(GetSymTP(cursym)));
	    argoff += GetTPSize(GetSymTP(cursym));
	    if ((GetTPSize(GetSymTP(cursym))) == 1)
		argoff += 1;
	    if (argoff % 2) {
		argoff++;
	    }
	}
    }
    return argoff;
}

void
Allocate2Locals(SymListVia_t table, InstListVia_t Codes)
/*
 * Assign location records to the local variables for a block.  All the
 * locals for all the blocks within a single function are allocated on the
 * same stack frame, using one link instruction at the beginning of the
 * function.
 */
{
    int								i;
    SpillSlotVia_t                  slot;
    LocAMVia_t                      other;
    SYMVia_t                        list;
    char                            thename[64];

    if (!table)
		return;

	if (Via(table)->count)
    for (i=1;i<=Via(table)->count;i++) {
		list = TableGetNum(table, i);
		if (list) {
		    if (Via(list)->storage_class == SCC_static) {
				AddStatic(list);
		    } else if (Via(list)->storage_class == SCC_extern) {
			#ifdef OLDMEM
			    HLock((Handle) list);
			#endif
			    GetSymName(list,thename);
			    if (GetTPFlags(Via(list)->TP) & ISPASCALMASK) {
					int                             ndx = 0;
					while (thename[ndx]) {
					    if (islower(thename[ndx]))
						thename[ndx] = toupper(thename[ndx]);
					    ndx++;
					}
			    }
			    if (gProject->itsOptions->bigGlobals) {
					Via(list)->M68kDef.Loc = BuildLargeGlobal(MakeUserLabel(thename));
					MakeLocGlobal(Via(list)->M68kDef.Loc);
			    } else {
					Via(list)->M68kDef.Loc = BuildARegLabelDisplace(5, MakeUserLabel(thename));
					MakeLocGlobal(Via(list)->M68kDef.Loc);
			    }
			    #ifdef OLDMEM
			    HUnlock((Handle) list);
			    #endif
			SetLocSZ(Via(list)->M68kDef.Loc, M68_TypeSize(GetSymTP(list)));
			if (isFloatingType(GetSymTP(list)))
			    SetLocIsFloat(Via(list)->M68kDef.Loc, 1);
			/* TODO Should generate an XREF here. */
		    } else if ((Via(list)->storage_class == SCC_register)) {
				if (isIntegralType(GetSymTP(list))) {
				    other = PermDataReg();
				    if (other) {
						SetLocSZ(other, M68_TypeSize(GetSymTP(list)));
						Via(list)->M68kDef.Loc = other;
				    } else {
						slot = GetLocalSlot(GetSymTP(list));
						Via(list)->M68kDef.Loc = Via(slot)->SlotLoc;
				    }
				} else if (isPointerType(GetSymTP(list))) {
				    other = PermAddrReg();
				    SetLocSZ(other, M68sz_long);
				    if (other) {
						Via(list)->M68kDef.Loc = other;
				    } else {
						slot = GetLocalSlot(GetSymTP(list));
						Via(list)->M68kDef.Loc = Via(slot)->SlotLoc;
				    }
				} else {
				    slot = GetLocalSlot(GetSymTP(list));
				    Via(list)->M68kDef.Loc = Via(slot)->SlotLoc;
				    if (isFloatingType(GetSymTP(list)))
						SetLocIsFloat(Via(list)->M68kDef.Loc, 1);
				}
		    } else {
				slot = GetLocalSlot(GetSymTP(list));
				Via(list)->M68kDef.Loc = Via(slot)->SlotLoc;
				if (isFloatingType(GetSymTP(list)))
				    SetLocIsFloat(Via(list)->M68kDef.Loc, 1);
		    }
		    if (Via(list)->Definition.Initializer) {
				LocAMVia_t                      res;
				LocAMVia_t                      postinc;
				int                             ndx;
				if (isArithmeticType(GetSymTP(list)) || isPointerType(GetSymTP(list))) {
				    res = GenExpression(Via(list)->Definition.Initializer, Codes);
				    Genmove(GetLocSZ(Via(list)->M68kDef.Loc), res, Via(list)->M68kDef.Loc,
					    Codes);
				    FreeTemp(res);
				} else {
				    InitDataAccIndex = 0;
				    ImageInit(GetSymTP(list), Via(list)->Definition.Initializer, Codes);
				    /*
				     * Now, move all the bytes in InitDataAcc[] to
				     * Via(list)->M68kDef.Loc
				     */
				    res = TempAddrReg(Codes);
				    GenLoadEA(Via(list)->M68kDef.Loc, res, Codes);
				    postinc = BuildARegPostInc(GetLocAReg(res));
				    ndx = 0;
				    while (ndx < GetTPSize(GetSymTP(list))) {
					if ((GetTPSize(GetSymTP(list)) - ndx) >= 4) {
					    GenInst(M68op_MOVE, M68sz_long,
						    BuildImmediate((*((long *) &(InitDataAcc[ndx]))), M68sz_long),
						    postinc, Codes);
					    ndx += 4;
					} else {
					    GenInst(M68op_MOVE, M68sz_word,
						    BuildImmediate((*((short *) &(InitDataAcc[ndx]))), M68sz_word),
						    postinc, Codes);
					    ndx += 2;
					}
				    }
				    FreeTemp(res);
				}
		    }
		}
    }
}

void
GenSymbol(SYMVia_t thesymbol, InstListVia_t Codes)
/*
 * General routine for generating the code for a global symbol. This symbol
 * could be external, static, function, variable, etc...
 */
{
    register SYMVia_t               symbol;
    int                             argoff;
    int                             argbytes = 0;
    if (NumErrors) return;
    symbol = thesymbol;
    if (symbol) {
	if (isFunctionType(GetSymTP(symbol))) {
	    if (Via(symbol)->Definition.FuncBody &&
		((Via(symbol)->storage_class != SCC_extern))) {
		InstVia_t                       last;
		LabSYMVia_t                     labsym;
		LabSYMVia_t                     routsym;
		int                             offset;
		if (GetSymFlags(symbol) & CODEOUTPUT) {
		    return;
		}
		/* Don't generate xdef if the routine is static. */
		if (Via(symbol)->storage_class != SCC_static) {
		    GenInst(M68op_XDEF, M68sz_none,
			    BuildLabelLoc(GetLocLabel(Via(symbol)->M68kDef.Loc)), NULL, Codes);
		}
		SetSymFlags(symbol, GetSymFlags(symbol) | CODEOUTPUT);
		FreeAllRegs();
		FreePerms();
		KillSpillList();
		SpillList = NULL;
		GenInst(M68op_CSEG, M68sz_none, NULL, NULL, Codes);
		offset = 0;
		labsym = FunctionLastLabel = MakeEccLabel(NextEccLabel++);
		routsym = GetLocLabel(Via(symbol)->M68kDef.Loc);
		GenInst(M68op_EVEN, M68sz_none, NULL, NULL, Codes);
		GenCodeLabel(routsym, Codes);

		/*
		 * The link instruction is initally generated with a link
		 * offset of 0, and the instruction record pointer is held in
		 * a global.  As blocks are encountered in the function, the
		 * local symbol table for each block is allocated, and this
		 * offset is changed directly through the pointer LinkInst.
		 */

		argoff = 8;	/* This is the offset for the first arg. */
		last = LinkInst = GenInst(M68op_LINK, M68sz_none,
					  BuildARegDirect(FRAME),
					  BuildImmediate(0,
							 M68sz_none), Codes);
		if (isStructUnionType(GetTPBase(GetSymTP(symbol)))) {
		    argoff += 4;
		    StructDoubleReturnLoc =
			BuildAutoLoc(8, M68_TypeSize(GetTPBase(GetSymTP(symbol))));
		    /*
		     * If this is a struct valued function, the last argument
		     * pushed on the stack is the pointer to the place to
		     * store the result.
		     */
		}
		if (GetTPFlags(GetSymTP(symbol)) & ISPASCALMASK) {
			if (GetTPMembers(GetSymTP(symbol))) {
			    /* Here we assign location records to each argument. */
			    argbytes = AllocatePascalParameters((GetTPMembers(GetSymTP(symbol))), argoff)
				- 8;	/* in this way, any hidden struct pointer arg
					 * is remove for pascal functions too. */
			}
			CurrentFunctionPascal = symbol;
			CurrentPascalArgs = argbytes;
		}
		else {
			CurrentFunctionPascal = NULL;
			if (GetTPMembers(GetSymTP(symbol))) {
			    /* Here we assign location records to each argument. */
			    argbytes = AllocateParameters((GetTPMembers(GetSymTP(symbol))), argoff)
				- 8;	/* in this way, any hidden struct pointer arg
					 * is remove for pascal functions too. */
			}
		}
		/* The actual code generation */
		GenExpression(Via(symbol)->Definition.FuncBody, Codes);

		/*
		 * Here, after the linkinst and before the rest of the
		 * function, if we use any non-temp registers, we need to
		 * push them onto the stack.
		 */

		PushPerms();

		GenLabel(labsym, Codes);

		/*
		 * And, here, if we used any nontemp regs, we need to pop
		 * them.
		 */

		PopPerms(Codes);

		last = GenInst(M68op_UNLK, M68sz_none,
			       BuildARegDirect(FRAME),
			       NULL, Codes);
		LinkInst = NULL;

		if (GetTPFlags(GetSymTP(symbol)) & ISPASCALMASK) {
		    /*
		     * Remove it's own arguments from the stack, and push its
		     * result, leaving the return address on top. The
		     * function pushed the arguments, and then the return
		     * address.  We must pop the return address, add to a7 to
		     * erase the args, push the result, and push the return
		     * address.
		     */
		    LocAMVia_t                      sp;
		    LocAMVia_t			    indA7;
		    LocAMVia_t                      a0;
		    LocAMVia_t                      stkpop;
		    LocAMVia_t                      stkpush;
		    sp = BuildARegDirect(7);
		    a0 = BuildARegDirect(0);
		    indA7 = BuildARegIndirect(7);
		    stkpop = BuildARegPostInc(7);
		    stkpush = BuildARegPreDec(7);
		    GenInst(M68op_MOVE, M68sz_long, stkpop, a0, Codes);
		    if (argbytes) {
			    GenInst(M68op_ADD, M68sz_long,
				    BuildImmediate(argbytes, M68sz_long), sp, Codes);
		    }
#ifdef Undefined
		    GenInst(M68op_MOVE, M68_TypeSize(GetTPBase(GetSymTP(symbol))),
			    BuildDRegLocation(0), indA7, Codes);
#endif
		    GenInst(M68op_JMP, M68sz_none, BuildARegIndirect(0), NULL, Codes);
#ifdef Undefined
			/* Changed 26 March - better pascal returns */
		    GenInst(M68op_MOVE, M68sz_long, a0, stkpush, Codes);
			GenInst(M68op_RTS, M68sz_none, NULL, NULL, Codes);
#endif
		}
		else {
			GenInst(M68op_RTS, M68sz_none, NULL, NULL, Codes);
		}
		if (gProject->itsOptions->MacsBugSymbols)
		    GenInst(M68op_MBG, M68sz_none, NULL, NULL, Codes);
		FreeTree(Via(symbol)->Definition.FuncBody);
		Via(symbol)->Definition.FuncBody = NULL;
	    } else {		/* No funcbody or extern */
		if (!(GetSymFlags(symbol) & CODEOUTPUT)) {
		    SetSymFlags(symbol, GetSymFlags(symbol) | CODEOUTPUT);
		    if (!Via(symbol)->Definition.FuncBody)
			GenInst(M68op_XREF, M68sz_none,
				BuildLabelLoc(GetLocLabel(Via(symbol)->M68kDef.Loc)), NULL, Codes);
		}
	    }
	} else {		/* Not a function */
	    if (Via(symbol)->storage_class == SCC_extern) {
		if (GetSymFlags(symbol) & REFOUTPUT) {
		    return;
		}
		SumGlobalSpace += GetTPSize(GetSymTP(symbol));
		SetSymFlags(symbol, GetSymFlags(symbol) | REFOUTPUT);
		GenInst(M68op_XREF, M68sz_none,
			BuildLabelLoc(GetLocLabel(Via(symbol)->M68kDef.Loc)), NULL, Codes);
	    } else {
		if (GetSymFlags(symbol) & CODEOUTPUT) {
		    return;
		}
		SumGlobalSpace += GetTPSize(GetSymTP(symbol));
		SetSymFlags(symbol, GetSymFlags(symbol) | CODEOUTPUT);
		SumGlobals += GetTPSize(GetSymTP(symbol));
		if (Via(symbol)->Definition.Initializer) {
			GenInst(M68op_DSEG, M68sz_none, NULL, NULL, Codes);
		    GenDataLabel(GetLocLabel(Via(symbol)->M68kDef.Loc), Codes);
		    GenInitGlobal(symbol, Codes);
		} else {
		    if (GetTPSize(GetSymTP(symbol)) == 1) {
			if (Via(symbol)->storage_class != SCC_static)
			    GenInst(M68op_COMM, M68sz_none,
				    BuildLabelLoc(GetLocLabel(Via(symbol)->M68kDef.Loc)),
				    BuildAbsolute(1), Codes);
			else
			    GenInst(M68op_LCOMM, M68sz_none,
				    BuildLabelLoc(GetLocLabel(Via(symbol)->M68kDef.Loc)),
				    BuildAbsolute(1), Codes);
			OddGlobal = !OddGlobal;
		    } else {
			if (OddGlobal) {
			    GenInst(M68op_EVEN, M68sz_none, NULL, NULL, Codes);
			    OddGlobal = 0;
			}
			if (Via(symbol)->storage_class != SCC_static)
			    GenInst(M68op_COMM, M68sz_none,
				    BuildLabelLoc(GetLocLabel(Via(symbol)->M68kDef.Loc)),
				    BuildAbsolute(GetTPSize(GetSymTP(symbol))), Codes);
			else
			    GenInst(M68op_LCOMM, M68sz_none,
				    BuildLabelLoc(GetLocLabel(Via(symbol)->M68kDef.Loc)),
				    BuildAbsolute(GetTPSize(GetSymTP(symbol))), Codes);
		    }
		}
		if (Via(symbol)->storage_class != SCC_static) {
		    GenInst(M68op_XDEF, M68sz_none,
			    BuildLabelLoc(GetLocLabel(Via(symbol)->M68kDef.Loc)), NULL, Codes);
		}
	    }
	}
    }
}
