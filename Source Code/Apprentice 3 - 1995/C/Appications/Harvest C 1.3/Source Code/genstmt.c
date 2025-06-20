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
 * statements.
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
extern SYMVia_t CurrentFunctionPascal;
extern long CurrentPascalArgs;

#pragma segment GenStmt


void
GenCompound(ParseTreeVia_t stmt, InstListVia_t Codes)
{
    /* Allocate the locals for this block. */
    Allocate2Locals((Via(GetTreeScopes(stmt))->Symbols), Codes);
    if (stmt) {
	GenExpression(Via(stmt)->a, Codes);
    }
    Free2Locals(Via(GetTreeScopes(stmt))->Symbols, Codes);
}

void
GenReturn(ParseTreeVia_t val, InstListVia_t Codes)
{
    LocAMVia_t                      temp;
    LocAMVia_t                      bigresult;
    SpillSlotVia_t                  slot;
    temp = GenExpression(val, Codes);
    if (temp) {
	if (isStructUnionType(GetTreeTP(val))) {
	    LocAMVia_t                      tloc;
	    tloc = TempAddrReg(Codes);
	    Genmove(M68sz_long, StructDoubleReturnLoc, tloc, Codes);
	    bigresult = BuildARegIndirect(GetLocAReg(tloc));
	    Genmove(M68_TypeSize(GetTreeTP(val)), temp, bigresult, Codes);
	    FreeTemp(tloc);
	} else if (isFloatingType(GetTreeTP(val))) {
	    /* Move the value into d0-d1-a0 */
	    if (gProject->itsOptions->useMC68881) {
		LocAMVia_t                      half2;
		slot = GetSpillSlot(SizeOfLongDouble);
		Genmove(M68_TypeSize(GetTreeTP(val)), temp, Via(slot)->SlotLoc, Codes);
		Genmove(M68sz_word, Via(slot)->SlotLoc, BuildDRegLocation(0),
			Codes);
		half2 = BuildARegDisplace(GetLocAReg(Via(slot)->SlotLoc),
				    GetLocConstant(Via(slot)->SlotLoc) + 2);
		Genmove(M68sz_long, half2, BuildDRegLocation(1), Codes);
		half2 = BuildARegDisplace(GetLocAReg(Via(slot)->SlotLoc),
				    GetLocConstant(Via(slot)->SlotLoc) + 6);
		Genmove(M68sz_long, half2, BuildARegDirect(0), Codes);
		FreeTemp(Via(slot)->SlotLoc);
	    } else {
		LocAMVia_t                      half2;
		LocAMVia_t                      val2;
		if (GetLocAM(temp) == M68am_FSANEtemp) {
		    val2 = Via(slot)->SlotLoc;
		} else
		    val2 = temp;
		Genmove(M68sz_word, val2, BuildDRegLocation(0), Codes);
		half2 = BuildARegDisplace(GetLocAReg(val2),
					  GetLocConstant(val2) + 2);
		Genmove(M68sz_long, half2, BuildDRegLocation(1), Codes);
		half2 = BuildARegDisplace(GetLocAReg(val2),
					  GetLocConstant(val2) + 6);
		Genmove(M68sz_long, half2, BuildARegDirect(0), Codes);
		FreeTemp(val2);
	    }
	} else {
	    Genmove(M68_TypeSize(GetTreeTP(val)),
		    temp, BuildDRegLocation(0), Codes);
	    /* If the function is pascal, then we move.sz d0,X(a6) */
	    if (CurrentFunctionPascal) {
	    	Genmove(M68_TypeSize(GetTreeTP(val)),BuildDRegLocation(0),
	    		BuildARegDisplace(6,CurrentPascalArgs+8),Codes);
	    }
	}
    }
    GenInst(M68op_BRA, M68sz_none,
	    BuildLabelLoc(FunctionLastLabel), NULL, Codes);
}

void
GenIfThenElse(ParseTreeVia_t expr,
	      ParseTreeVia_t thenstmt,
	      ParseTreeVia_t elsestmt,
	      InstListVia_t Codes)
{
    LabSYMVia_t                     ElseLabel = NULL;
    LabSYMVia_t                     EndLabel;
    FreeAllRegs();
    EndLabel = MakeEccLabel(NextEccLabel++);
    if (elsestmt) {
	ElseLabel = MakeEccLabel(NextEccLabel++);
	GenCompareZero(expr, ElseLabel, Codes);
    } else {
	GenCompareZero(expr, EndLabel, Codes);
    }
    GenExpression(thenstmt, Codes);
    if (elsestmt) {
	GenInst(M68op_BRA, M68sz_none,
		BuildLabelLoc(EndLabel), NULL, Codes);
	GenLabel(ElseLabel, Codes);
	GenExpression(elsestmt, Codes);
    }
    GenLabel(EndLabel, Codes);
}

void
GenDoWhile(ParseTreeVia_t stmt,
	   ParseTreeVia_t expr,
	   InstListVia_t Codes)
{
    LabSYMVia_t                     OldBreak;
    LabSYMVia_t                     OldContinue;
    LocAMVia_t                      exprloc;
    FreeAllRegs();
    OldBreak = BreakLabel;
    OldContinue = ContinueLabel;
    ContinueLabel = MakeEccLabel(NextEccLabel++);
    BreakLabel = MakeEccLabel(NextEccLabel++);
    GenLabel(ContinueLabel, Codes);
    exprloc = GenExpression(stmt, Codes);
    GenCompareNonZero(expr, ContinueLabel, Codes);
    GenLabel(BreakLabel, Codes);
    BreakLabel = OldBreak;
    ContinueLabel = OldContinue;
}

void
Genforstmt(ParseTreeVia_t first,
	   ParseTreeVia_t cond,
	   ParseTreeVia_t each,
	   ParseTreeVia_t stmt,
	   InstListVia_t Codes)
{
    LabSYMVia_t                     OldBreak;
    LabSYMVia_t                     OldContinue;
    FreeAllRegs();
    /* Save break and continue for nested loops */
    OldBreak = BreakLabel;
    OldContinue = ContinueLabel;
    ContinueLabel = MakeEccLabel(NextEccLabel++);
    BreakLabel = MakeEccLabel(NextEccLabel++);
    GenExpression(first, Codes);
    GenLabel(ContinueLabel, Codes);
    if (cond)
    	GenCompareZero(cond, BreakLabel, Codes);
    GenExpression(stmt, Codes);
    GenExpression(each, Codes);
    GenInst(M68op_BRA, M68sz_none,
	    BuildLabelLoc(ContinueLabel), NULL, Codes);
    GenLabel(BreakLabel, Codes);
    BreakLabel = OldBreak;
    ContinueLabel = OldContinue;
}

void
GenWhile(ParseTreeVia_t expr,
	 ParseTreeVia_t stmt,
	 InstListVia_t Codes)
{
    /*
     * Here, we generate a starting label.  Then, we generate code to
     * evaluate the conditional expression.  Then, we generate a comparioson
     * and a conditional branch.  If the expression is true, we do not
     * branch.  Execution drops down. If it is false, we branch to the ending
     * label.  These labels must be local variables, because loops may be
     * nested.  We then generate code to evaluate the statement.  Then, a BRA
     * to the starting label. The ending label goes on the instruction just
     * after the bra.
     */
    LabSYMVia_t                     OldBreak;
    LabSYMVia_t                     OldContinue;
    FreeAllRegs();
    OldBreak = BreakLabel;
    OldContinue = ContinueLabel;
    ContinueLabel = MakeEccLabel(NextEccLabel++);
    BreakLabel = MakeEccLabel(NextEccLabel++);
    GenLabel(ContinueLabel, Codes);
    GenCompareZero(expr, BreakLabel, Codes);
    GenExpression(stmt, Codes);
    GenInst(M68op_BRA, M68sz_none,
	    BuildLabelLoc(ContinueLabel), NULL, Codes);
    GenLabel(BreakLabel, Codes);
    BreakLabel = OldBreak;
    ContinueLabel = OldContinue;
}

void
GenLabelledStmt(ParseTreeVia_t expr, InstListVia_t Codes)
{
    LabSYMVia_t                     lab;
    char                            nm[64];
    GetAbsString(ParserStrings, Via(expr)->data.identifier, nm);
    lab = AddLabel(nm);
    GenLabel(lab, Codes);
    GenExpression(Via(expr)->a, Codes);
}

void
GenGotoStmt(ParseTreeVia_t stmt, InstListVia_t Codes)
{
    LabSYMVia_t                     lab;
    char                            nm[64];
    GetAbsString(ParserStrings, Via(stmt)->data.identifier, nm);
    lab = AddLabel(nm);
    GenInst(M68op_BRA, M68sz_none, BuildLabelLoc(lab), NULL, Codes);
}

LabSYMVia_t                     LabTableGetNum(LabSymListVia_t, int);

void
GenSWITCH(ParseTreeVia_t expr, ParseTreeVia_t stmt,
	  InstListVia_t Codes)
{
    /*
     * Here we will generate a large set of if-elses. First we go through the
     * stmt, counting labels.  This stmt has a set of case label: stmt;
     * lines.  We must generate a test for every case in the group, but we
     * only need to generate an assembler label for each actual statement.
     * So, first, we generate a bra to the tests, a label to be defined
     * later.  Then, we go through and generate code for each statement in
     * stmt, with labels for every case.  The break label is set to the end
     * of the switch.  After generating the cases, we generate the test and
     * branch section, jumping to the labels already defined.
     */

    LabSYMVia_t                     TestsLabel;
    LabSYMVia_t                     OldBreak;
    LabSymListVia_t                 OldCases;
    LabSYMVia_t                     cur;
    int                             ndx;
    LocAMVia_t                      control;
    OldCases = SwitchCases;
    SwitchCases = RawLabTable();
    OldBreak = BreakLabel;
    BreakLabel = MakeEccLabel(NextEccLabel++);
    TestsLabel = MakeEccLabel(NextEccLabel++);

    GenInst(M68op_BRA, M68sz_none, BuildLabelLoc(TestsLabel), NULL, Codes);
    GenExpression(stmt, Codes);
    GenInst(M68op_BRA, M68sz_none, BuildLabelLoc(BreakLabel), NULL, Codes);
    /* Now we generate the tests. */
    GenLabel(TestsLabel, Codes);

    control = GenExpression(expr, Codes);
    if (isFloatingType(GetTreeTP(expr))) {
	control = DictateTempFloat(control, Codes);
    }
    else {
	control = DictateAnyDReg(control, Codes);
    }

    ndx = Via(SwitchCases)->count;
    ndx = 1;
    while (ndx <= Via(SwitchCases)->count) {
	cur = LabTableGetNum(SwitchCases, ndx);
	ndx++;
	if (Via(cur)->Definition.CaseConstant) {
	    GenCompareBranch2(control, Via(cur)->Definition.CaseConstant, M68op_BEQ,
			      cur, Codes);
	} else {
	    GenInst(M68op_BRA, M68sz_none, BuildLabelLoc(cur), NULL, Codes);
	}
    }
    FreeTemp(control);
    GenLabel(BreakLabel, Codes);
    BreakLabel = OldBreak;
    RememberSwitch(SwitchCases);
    SwitchCases = OldCases;
}

void
GenSwitchDefault(ParseTreeVia_t stmt, InstListVia_t Codes)
{
    /*
     * Generate a label.  Then generate the statement.  Save the symbol
     * record for the label, and store the constant expression from the case
     * statement in that symbol record.  Add that symbol record to the table
     * SwitchCases.
     */
    LabSYMVia_t                     caselabel;
    caselabel = MakeEccLabel(NextEccLabel++);
#ifdef OLDMEM
    HLock((Handle) caselabel);
#endif
    caselabel = LabTableTailAdd(SwitchCases, Via(caselabel)->name);
#ifdef OLDMEM
    HUnlock((Handle) caselabel);
#endif
    Via(caselabel)->Definition.CaseConstant = NULL;
    GenLabel(caselabel, Codes);
    GenExpression(Via(stmt)->a, Codes);
}

void
GenSwitchCase(ParseTreeVia_t stmt, InstListVia_t Codes)
{
    /*
     * Generate a label.  Then generate the statement.  Save the symbol
     * record for the label, and store the constant expression from the case
     * statement in that symbol record.  Add that symbol record to the table
     * SwitchCases.
     */
    LabSYMVia_t                     caselabel;
    caselabel = MakeEccLabel(NextEccLabel++);
    caselabel = LabTableTailAdd(SwitchCases, Via(caselabel)->name);
    Via(caselabel)->Definition.CaseConstant = Via(stmt)->a;
    GenLabel(caselabel, Codes);
    GenExpression(Via(stmt)->b, Codes);
}

