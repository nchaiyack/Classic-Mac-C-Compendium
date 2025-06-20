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
 * This file contains routines for management of symbol tables.  Symbol tables
 * are used very heavily in Harvest C.  They manage everything from
 * preprocessor symbols to string literals, and of course, the C identifiers.
 * 
 * 
 * 
 */

#include "conditcomp.h"
#include <stdio.h>
#include <string.h>

#include "structs.h"

#include "SymTable.h"

#pragma segment SymbolLists

SymListVia_t
RawTable(int tablesize)
/* Create an empty, unused Symbol Table. */
{
    register SymListVia_t           raw;
    raw = Ealloc(sizeof(SymList_t));
    if (tablesize) {
		Via(raw)->hashtable = Ealloc(tablesize * sizeof(Ptr));
		#ifdef OLDMEM
		HLock((Handle) Via(raw)->hashtable);
		#endif
		memset(Via(Via(raw)->hashtable), 0L, (tablesize * sizeof(Ptr)));
		#ifdef OLDMEM
		HUnlock((Handle) Via(raw)->hashtable);
		#endif
	} else
		Via(raw)->hashtable = NULL;
    Via(raw)->count = 0;
    Via(raw)->isShadow = 0;
    Via(raw)->tablesize = tablesize;
    return raw;
}

#ifdef KILLTHIS
unsigned long
TableHash(char *name)
/* This is the hash function for the symbol tables. */
{
    unsigned long                   h = 0;
    unsigned long                   g;

    for (; *name; ++name) {
	h = (h << 4) + *name;
	if (g = h & 0xf0000000)
	    h = (h ^ (g >> 24)) & 0x0fffffff;
    }
    return h;
}

#endif

unsigned long
TableHash(char *name)
/* This is the hash function for the symbol tables. */
{
    unsigned long                   h = 0;
    unsigned long                   g;

    for (; *name; ++name) {
	h = h * 65599 + *name;
    }
    if (!h)
	h = 1;
    return h;
}

SYMVia_t
TableAdd(SymListVia_t table, char *name)
/*
 * General routine for adding a symbol to a symbol table.  The symbol table
 * data structures are used for many purposes.  Note that currently, a number
 * of routines depend on these tables being kept in order.
 */
{
    /*
     * All the table manipulation routines may be modified later to make the
     * tables vastly more efficient.
     */
    register SYMVia_t               tmp;
    register SYMVia_t               tmp2, prev;
    register unsigned long          hk;
    tmp = RawSymbol(name);
    hk = TableHash(name) % Via(table)->tablesize;
    assert(table);
    Via(tmp)->IndexInserted = ++Via(table)->count;
    Via(tmp)->next = Via(Via(table)->hashtable)[hk];
    Via(Via(table)->hashtable)[hk] = tmp;
    return tmp;
}

int
TableSearchNum(SymListVia_t table, char *name)
/*
 * This routine searches a table for the given name, returning the index of
 * the symbol record for that name if it was found, otherwise NULL.
 */
{
    register SYMVia_t               tmp;
    tmp = TableSearch(table, name);
    if (tmp)
	return Via(tmp)->IndexInserted;
    else
	return 0;
}

SYMVia_t
TableGetNum(SymListVia_t table, int num)
/*
 * This table returns symbol record # num from the given table.  If the table
 * does not contains num records, then NULL.  The first record in a table is
 * numbered 1.
 */
{
    SYMVia_t                        tmp;
    int                             ndx;
    if (num > Via(table)->count)
	return NULL;
    ndx = 0;
    while (ndx < Via(table)->tablesize) {
	tmp = Via(Via(table)->hashtable)[ndx++];
	while (tmp) {
	    if (Via(tmp)->IndexInserted == num)
		return tmp;
	    tmp = Via(tmp)->next;
	}
    }
    return NULL;
}

SYMVia_t
TableSearch(SymListVia_t table, char *name)
/*
 * This routine searches a table for the given name, returning the symbol
 * record for that name if it was found, otherwise NULL.
 */
{
    register SYMVia_t               tmp;
    unsigned long                   hashval;
    if (!Via(table)->count) return 0;
    hashval = TableHash(name) % Via(table)->tablesize;
    tmp = Via(Via(table)->hashtable)[hashval];
    while (tmp) {
	if (!strcmp(Via(tmp)->name, name)) {
	    return tmp;
	} else
	    tmp = Via(tmp)->next;
    }
    return NULL;
}

SYMVia_t
TableRemove(SymListVia_t table, char *name)
/* Removes a given symbol from the table. */
{
    register SYMVia_t               tmp;
    tmp = TableSearch(table, name);
    if (tmp) {
	Via(tmp)->name[0] = 0;
    }
    return NULL;
}

SymListVia_t
CopySymTable(SymListVia_t table)
{
    SymListVia_t                    copy = NULL;
    if (table) {
	copy = RawTable(0);
	Via(copy)->hashtable = Via(table)->hashtable;
	Via(copy)->count = Via(table)->count;
	Via(copy)->isShadow = 1;
    }
    return copy;
}

int
MaxSizes(SymListVia_t table)
/* This is used for calculating the size of a union */
{
    int                             ndx;
    SYMVia_t                        tmp;
    unsigned long                   num = 0;
    ndx = 0;
    while (ndx < Via(table)->tablesize) {
	tmp = Via(Via(table)->hashtable)[ndx++];
	while (tmp) {
	    Via(tmp)->numbers.structoffset = 0;
	    if (Via(tmp)->TP) {
		if (GetTPSize(Via(tmp)->TP) > num) {
		    num = GetTPSize(Via(tmp)->TP);
		}
	    }
	    tmp = Via(tmp)->next;
	}
    }
    return num;
}

SYMVia_t
FirstSym(SymListVia_t tbl)
{
    return TableGetNum(tbl, 1);
}

int
SameTable(SymListVia_t a, SymListVia_t b)
{
    /*
     * This method assumes that there is ONLY one copy of the member list
     * symbol table for a struct/union record, and we can simply compare the
     * pointer.  Same goes for functions and enums.
     */

    int                             ndx;
    SYMVia_t                        a1;
    SYMVia_t                        b1;

    if (a == b) {
	return 1;
    }
    if (a && b) {
	if (Via(a)->count != Via(b)->count) {
	    return 0;
	}
	ndx = 1;
	while (ndx <= Via(a)->count) {
	    a1 = TableGetNum(a, ndx);
	    b1 = TableGetNum(b, ndx);
	    if (!SameType(Via(a1)->TP, Via(b1)->TP)) {
		return 0;
	    }
	    ndx++;
	}
	return 1;
    } else {
	VeryBadParseError("NULL arg in SameTable...");
	return 0;
    }
}

void
FreeSyms(SYMVia_t cur, int dodef)
{
    if (cur) {
	FreeSyms(Via(cur)->next, dodef);
	if (dodef)
	    FreeTree(Via(cur)->Definition.FuncBody);
	/* M68kDef ??? */
	Efree(cur);
    }
}

void
FreeSymbolList(SymListVia_t table, int dodef)
{
    SYMVia_t                        cur;
    SYMVia_t                        next;
    if (table) {
	int                             ndx;
	ndx = 0;
	if (Via(table)->isShadow)
	    return;
	while (ndx < Via(table)->tablesize) {
	    FreeSyms(Via(Via(table)->hashtable)[ndx++], dodef);
	}
	Efree(Via(table)->hashtable);
	Efree(table);
    }
}

void
CheckUsages(SymListVia_t table)
{
    int                             ndx;
    SYMVia_t                        tmp;
    ndx = 0;
    while (ndx < Via(table)->tablesize) {
	tmp = Via(Via(table)->hashtable)[ndx++];
	while (tmp) {
	    if (!Via(tmp)->numbers.CountUses) {
	    #ifdef OLDMEM
		HLock((Handle) tmp);
		#endif
		UserWarning2(WARN_unusedvariable, Via(tmp)->name);
		#ifdef OLDMEM
		HUnlock((Handle) tmp);
		#endif
	    }
	    tmp = Via(tmp)->next;
	}
    }
}

int
GenGlobal(SymListVia_t table, InstListVia_t Codes, int Flush)
{
    int                             ndx;
    SYMVia_t                        tmp;
    /*
     * Given a symbol table, generate code for each symbol into the table.
     */
    if (!table) return 0;
    if (!Via(table)->count) return 0;
    FreeAllRegs();		/* Is this necessary ? */
    /* Then, we generate code for everything. */
    ndx = 0;
    while (ndx < Via(table)->tablesize) {
	tmp = Via(Via(table)->hashtable)[ndx++];
	while (tmp) {
	    if (Flush) {
		GenSymbol(tmp, Codes);
	    } else {
		if (Via(tmp)->storage_class != SCC_extern) {
		    GenSymbol(tmp, Codes);
		}
	    }
	    tmp = Via(tmp)->next;
	}
    }
    GenSegments(Codes);
    GenStringLits(Codes);
    GenFloatLits(Codes);
    GenStatics(Codes);

    return Via(table)->count;
}

void
GenStringLits(InstListVia_t Codes)
{
    int                             ndx;
    SYMVia_t                        tmp;
    ndx = 0;
    if (Via(StringLits)->count) {
	    while (ndx < Via(StringLits)->tablesize) {
		tmp = Via(Via(StringLits)->hashtable)[ndx++];
			while (tmp) {
			    GenOneLit(tmp, Codes);
			    tmp = Via(tmp)->next;
			}
	    }
    }
}

void
GenStatics(InstListVia_t Codes)
/* Loop to generate all the static variables. */
{
    int                             ndx;
    SYMVia_t                        tmp;
    ndx = 0;
    if (Via(StaticTable)->count) {
	    while (ndx < Via(StaticTable)->tablesize) {
		tmp = Via(Via(StaticTable)->hashtable)[ndx++];
		while (tmp) {
		    if (!(Via(tmp)->SymbolFlags & CODEOUTPUT)) {
				GenInst(M68op_DSEG, M68sz_none, NULL, NULL, Codes);
				Via(tmp)->SymbolFlags |= CODEOUTPUT;
				GenDataLabel(GetLocLabel(Via(tmp)->M68kDef.Loc), Codes);
				GenInst(M68op_DS, M68sz_byte,
					BuildAbsolute(GetTPSize(Via(tmp)->TP)), NULL, Codes);
		    }
		    tmp = Via(tmp)->next;
		}
	    }
    }
}

void
Free2Locals(SymListVia_t list, InstListVia_t Codes)
{
    int                             ndx;
    SYMVia_t                        tmp;
    ndx = 0;
    while (ndx < Via(list)->tablesize) {
	tmp = Via(Via(list)->hashtable)[ndx++];
	while (tmp) {
	    FreeIt(Via(tmp)->M68kDef.Loc);
	    tmp = Via(tmp)->next;
	}
    }
}

TypeRecordVia_t
GetSymTP(SYMVia_t s)
{
    assert(s);
    return Via(s)->TP;
}

void
SetSymTP(SYMVia_t s, TypeRecordVia_t TP)
{
    assert(s);
    Via(s)->TP = TP;
}

void
GetSymName(SYMVia_t s, char *nm)
{
    assert(s);
    assert(nm);
    strcpy(nm, Via(s)->name);
}

void
SetSymFlags(SYMVia_t s, unsigned short f)
{
    assert(s);
    Via(s)->SymbolFlags = f;
}

unsigned short
GetSymFlags(SYMVia_t s)
{
    assert(s);
    return Via(s)->SymbolFlags;
}
